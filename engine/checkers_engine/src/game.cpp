#include "game.hpp"

#include <regex>
#include <utility>
#include <vector>
#include <cstddef>
#include <cstring>
#include <cassert>

#include "error.hpp"
#include "search_node.hpp"

namespace game {
    static std::vector<std::string> split(const std::string& string, const char* delimiter) {
        std::vector<std::string> tokens;
        std::string buffer {string};

        char* token {std::strtok(buffer.data(), delimiter)};

        while (token != nullptr) {
            tokens.emplace_back(token);
            token = std::strtok(nullptr, delimiter);
        }

        return tokens;
    }

    static bool valid_fen_string(const std::string& fen_string) {
        const std::regex pattern {"(W|B)(:(W|B)(K?[0-9]+(,K?[0-9]+){0,11})?){2}"};

        return std::regex_match(fen_string, pattern);
    }

    static bool valid_move_string(const std::string& move_string) {
        const std::regex pattern {"([0-9]+(x|-))+[0-9]+"};

        return std::regex_match(move_string, pattern);
    }

    static Player parse_player_type(const std::string& string) {
        if (string == "B") {
            return Player::Black;
        } else if (string == "W") {
            return Player::White;
        } else {
            throw error::InvalidCommand();
        }
    }

    static std::pair<Idx, Square> parse_player_piece(const std::string& string, Player player_type) {
        const bool king {string.at(0) == 'K'};

        try {
            switch (player_type) {
                case Player::Black:
                    if (king) {
                        return std::make_pair(static_cast<Idx>(std::stoi(string.substr(1))), Square::BlackKing);
                    } else {
                        return std::make_pair(static_cast<Idx>(std::stoi(string)), Square::Black);
                    }
                case Player::White:
                    if (king) {
                        return std::make_pair(static_cast<Idx>(std::stoi(string.substr(1))), Square::WhiteKing);
                    } else {
                        return std::make_pair(static_cast<Idx>(std::stoi(string)), Square::White);
                    }
            }
        } catch (...) {  // stoi
            throw error::InvalidCommand();
        }

        return {};
    }

    static std::vector<std::pair<Idx, Square>> parse_player_pieces(const std::string& string) {
        const Player player_type {parse_player_type(string.substr(0, 1))};

        const auto tokens {split(string.substr(1), ",")};

        std::vector<std::pair<Idx, Square>> pieces;

        for (const auto& token : tokens) {
            pieces.push_back(parse_player_piece(token, player_type));
        }

        for (const auto& [index, _] : pieces) {
            if (index < 1 || index > 32) {
                throw error::InvalidCommand();
            }
        }

        return pieces;
    }

    static std::pair<Board, Player> parse_fen_string(const std::string& fen_string) {
        const auto tokens {split(fen_string, ":")};

        if (tokens.size() != 3) {
            throw error::InvalidCommand();
        }

        if (tokens.at(1).at(0) == tokens.at(2).at(0)) {
            throw error::InvalidCommand();
        }

        const Player turn {parse_player_type(tokens.at(0))};
        const auto pieces1 {parse_player_pieces(tokens.at(1))};
        const auto pieces2 {parse_player_pieces(tokens.at(2))};

        Board board {};

        for (const auto& [index, square] : pieces1) {
            board[_1_32_to_0_31(index)] = square;
        }

        for (const auto& [index, square] : pieces2) {
            board[_1_32_to_0_31(index)] = square;
        }

        return std::make_pair(board, turn);
    }

    static bool is_capture_move(Idx source, Idx destination) {
        // Indices must be in the range [1, 32]

        const auto distance {std::abs(_1_32_to_0_31(source) - _1_32_to_0_31(destination))};

        if (distance >= 3 && distance <= 5) {
            return false;
        } else if (distance == 7 || distance == 9) {
            return true;
        } else {
            assert(false);
            return {};
        }
    }

    static std::vector<Idx> parse_squares(const std::string& string) {
        const auto tokens {split(string, "x")};

        std::vector<Idx> squares;

        try {
            for (const auto& token : tokens) {
                squares.push_back(static_cast<Idx>(std::stoi(token)));
            }
        } catch (...) {  // stoi
            throw error::InvalidCommand();
        }

        for (const int square : squares) {
            if (!(square >= 1 && square <= 32)) {
                throw error::InvalidCommand();
            }
        }

        return squares;
    }

    static Idx get_jumped_piece_index(Idx index1, Idx index2) {
        // This works with indices in the range [1, 32]

        const int sum {index1 + index2};

        assert(sum % 2 == 1);

        if (((_1_32_to_0_31(index1)) / 4) % 2 == 0) {
            return static_cast<Idx>((sum + 1) / 2);
        } else {
            return static_cast<Idx>((sum - 1) / 2);
        }
    }

    static void remove_jumped_pieces(Board& board, const Move& move) {
        assert(move.type == MoveType::Capture);

        {
            assert(
                board[move.capture.destination_indices[0]] == Square::None ||
                move.capture.source_index == move.capture.destination_indices[0]
            );

            const auto index {get_jumped_piece_index(
                _0_31_to_1_32(move.capture.source_index),
                _0_31_to_1_32(move.capture.destination_indices[0])
            )};

            assert(board[_1_32_to_0_31(index)] != Square::None);

            board[_1_32_to_0_31(index)] = Square::None;
        }

        for (unsigned char i {0}; i < move.capture.destination_indices_size - 1; i++) {
            assert(
                board[move.capture.destination_indices[i + 1]] == Square::None ||
                move.capture.source_index == move.capture.destination_indices[i + 1]
            );

            const auto index {get_jumped_piece_index(
                _0_31_to_1_32(move.capture.destination_indices[i]),
                _0_31_to_1_32(move.capture.destination_indices[i + 1])
            )};

            assert(board[_1_32_to_0_31(index)] != Square::None);

            board[_1_32_to_0_31(index)] = Square::None;
        }
    }

    static void check_piece_crowning(Board& board, Idx square_index) {
        const int row {square_index / 4};

        if (is_black_piece(board[square_index])) {  // TODO opt.
            if (row == 7) {
                board[square_index] = Square::BlackKing;
            }
        } else {  // Assume it's white
            assert(is_white_piece(board[square_index]));

            if (row == 0) {
                board[square_index] = Square::WhiteKing;
            }
        }
    }

    bool Move::operator==(const Move& other) const {
        if (type != other.type) {
            return false;
        }

        switch (type) {
            case MoveType::Normal:
                if (normal.source_index != other.normal.source_index) {
                    return false;
                }

                if (normal.destination_index != other.normal.destination_index) {
                    return false;
                }

                break;
            case MoveType::Capture:
                if (capture.source_index != other.capture.source_index) {
                    return false;
                }

                if (capture.destination_indices_size != other.capture.destination_indices_size) {
                    return false;
                }

                for (unsigned char i {0}; i < capture.destination_indices_size; i++) {
                    if (capture.destination_indices[i] != other.capture.destination_indices[i]) {
                        return false;
                    }
                }

                break;
        }

        return true;
    }

    void set_position(Position& position, const std::string& fen_string) {
        if (!valid_fen_string(fen_string)) {
            throw error::InvalidCommand();
        }

        const auto [board, turn] {parse_fen_string(fen_string)};

        position.board = board;
        position.player = turn;
    }

    void play_move(Position& position, const std::string& move_string) {
        if (!valid_move_string(move_string)) {
            throw error::InvalidCommand();
        }

        const Move move {parse_move_string(move_string)};

        play_move(position, move);
    }

    void play_move(Position& position, const Move& move) {
        auto& board {position.board};

        switch (move.type) {
            case MoveType::Normal:
                assert(board[move.normal.source_index] != Square::None);
                assert(board[move.normal.destination_index] == Square::None);

                std::swap(board[move.normal.source_index], board[move.normal.destination_index]);

                if (!is_king_piece(board[move.normal.destination_index])) {
                    position.plies_without_advancement = 0;
                } else {
                    position.plies_without_advancement++;
                }

                check_piece_crowning(board, move.normal.destination_index);

                break;
            case MoveType::Capture:
                assert(board[move.capture.source_index] != Square::None);
                assert(
                    board[move.capture.destination_indices[move.capture.destination_indices_size - 1]] == Square::None ||
                    move.capture.source_index == move.capture.destination_indices[move.capture.destination_indices_size - 1]
                );

                remove_jumped_pieces(board, move);
                std::swap(
                    board[move.capture.source_index],
                    board[move.capture.destination_indices[move.capture.destination_indices_size - 1]]
                );

                position.plies_without_advancement = 0;

                check_piece_crowning(board, move.capture.destination_indices[move.capture.destination_indices_size - 1]);

                break;
        }

        position.player = opponent(position.player);
        position.plies++;
    }

    void play_move(search::SearchNode& node, const Move& move) {
        switch (move.type) {
            case MoveType::Normal:
                assert(node.board[move.normal.source_index] != Square::None);
                assert(node.board[move.normal.destination_index] == Square::None);

                std::swap(node.board[move.normal.source_index], node.board[move.normal.destination_index]);

                if (!is_king_piece(node.board[move.normal.destination_index])) {
                    node.plies_without_advancement = 0;
                    node.previous = nullptr;
                } else {
                    node.plies_without_advancement++;
                }

                check_piece_crowning(node.board, move.normal.destination_index);

                break;
            case MoveType::Capture:
                assert(node.board[move.capture.source_index] != Square::None);
                assert(
                    node.board[move.capture.destination_indices[move.capture.destination_indices_size - 1]] == Square::None ||
                    move.capture.source_index == move.capture.destination_indices[move.capture.destination_indices_size - 1]
                );

                remove_jumped_pieces(node.board, move);
                std::swap(
                    node.board[move.capture.source_index],
                    node.board[move.capture.destination_indices[move.capture.destination_indices_size - 1]]
                );

                node.plies_without_advancement = 0;
                node.previous = nullptr;

                check_piece_crowning(node.board, move.capture.destination_indices[move.capture.destination_indices_size - 1]);

                break;
        }

        node.player = opponent(node.player);
        node.plies++;
    }

    Move parse_move_string(const std::string& move_string) {
        // These are in the range [1, 32]
        const auto squares {parse_squares(move_string)};

        if (squares.size() < 2) {
            throw error::InvalidCommand();
        }

        Move move;

        if (is_capture_move(squares.at(0), squares.at(1))) {
            move.type = MoveType::Capture;
            move.capture.source_index = _1_32_to_0_31(squares.at(0));
            move.capture.destination_indices_size = static_cast<unsigned char>(squares.size() - 1);

            for (std::size_t i {0}; i < squares.size() - 1; i++) {
                move.capture.destination_indices[i] = _1_32_to_0_31(squares.at(i + 1));
            }
        } else {
            move.type = MoveType::Normal;
            move.normal.source_index = _1_32_to_0_31(squares.at(0));
            move.normal.destination_index = _1_32_to_0_31(squares.at(1));
        }

        return move;
    }

    Player opponent(Player player) {
        if (player == Player::Black) {  // TODO opt. maybe do xor
            return Player::White;
        } else {
            return Player::Black;
        }
    }

    bool is_move_advancement(const Board& board, const Move& move) {
        // Must be called right before the move has been played on the board

        if (move.type == MoveType::Normal) {
            return !is_king_piece(board[move.normal.source_index]);
        } else {
            return true;
        }
    }
}
