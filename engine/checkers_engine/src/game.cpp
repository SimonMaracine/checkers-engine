#include "game.hpp"

#include <regex>
#include <utility>
#include <vector>
#include <cstddef>
#include <cstring>
#include <cassert>

#include "error.hpp"
#include "search_node.hpp"
#include "utils.hpp"

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

    static std::pair<int, Square> parse_player_piece(const std::string& string, Player player) {
        const bool king {string.at(0) == 'K'};

        try {
            switch (player) {
                case Player::Black:
                    if (king) {
                        return std::make_pair(std::stoi(string.substr(1)), Square::BlackKing);
                    } else {
                        return std::make_pair(std::stoi(string), Square::Black);
                    }
                case Player::White:
                    if (king) {
                        return std::make_pair(std::stoi(string.substr(1)), Square::WhiteKing);
                    } else {
                        return std::make_pair(std::stoi(string), Square::White);
                    }
            }
        } catch (...) {  // stoi
            throw error::InvalidCommand();
        }

        return {};
    }

    static std::vector<std::pair<int, Square>> parse_player_pieces(const std::string& string) {
        const Player player_type {parse_player_type(string.substr(0, 1))};

        const auto tokens {split(string.substr(1), ",")};

        std::vector<std::pair<int, Square>> pieces;

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

    static std::vector<int> parse_squares(const std::string& string) {
        const auto tokens {split(string, "x-")};

        std::vector<int> squares;

        try {
            for (const auto& token : tokens) {
                squares.push_back(std::stoi(token));
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

    static bool is_capture_move(int source, int destination) noexcept {
        // Indices must be in the range [1, 32]

        const int distance {std::abs(_1_32_to_0_31(source) - _1_32_to_0_31(destination))};

        if (distance >= 3 && distance <= 5) {
            return false;
        } else if (distance == 7 || distance == 9) {
            return true;
        } else {
            assert(false);
            utils::unreachable();
        }
    }

    static int get_jumped_piece_index(int index1, int index2) noexcept {
        // This works with indices in the range [1, 32]

        const int sum {index1 + index2};

        assert(sum % 2 == 1);

        if (((_1_32_to_0_31(index1)) / 4) % 2 == 0) {
            return (sum + 1) / 2;
        } else {
            return (sum - 1) / 2;
        }
    }

    static void remove_jumped_pieces(Board& board, Move move) noexcept {
        assert(move.type() == MoveType::Capture);

        {
            assert(
                board[move.destination_index()] == Square::None ||
                move.source_index() == move.destination_index()
            );

            const auto index {get_jumped_piece_index(
                _0_31_to_1_32(move.source_index()),
                _0_31_to_1_32(move.destination_index())
            )};

            assert(board[_1_32_to_0_31(index)] != Square::None);

            board[_1_32_to_0_31(index)] = Square::None;
        }

        for (int i {0}; i < move.destination_indices_size() - 1; i++) {
            assert(
                board[move.destination_index(i + 1)] == Square::None ||
                move.source_index() == move.destination_index(i + 1)
            );

            const auto index {get_jumped_piece_index(
                _0_31_to_1_32(move.destination_index(i)),
                _0_31_to_1_32(move.destination_index(i + 1))
            )};

            assert(board[_1_32_to_0_31(index)] != Square::None);

            board[_1_32_to_0_31(index)] = Square::None;
        }
    }

    static void check_piece_crowning(Board& board, int square_index, Player player) noexcept {
        const int row {square_index / 4};

        switch (player) {
            case Player::Black:
                if (row == 7) {
                    board[square_index] = Square::BlackKing;
                }
                break;
            case Player::White:
                if (row == 0) {
                    board[square_index] = Square::WhiteKing;
                }
                break;
        }
    }

    Move parse_move_string(const std::string& move_string) {
        // These are in the range [1, 32]
        const auto squares {parse_squares(move_string)};

        if (squares.size() < 2) {
            throw error::InvalidCommand();
        }

        if (is_capture_move(squares.at(0), squares.at(1))) {
            Move::DestinationIndices destination_indices {};

            for (std::size_t i {0}; i < squares.size() - 1; i++) {
                destination_indices[i] = _1_32_to_0_31(squares.at(i + 1));
            }

            return Move(_1_32_to_0_31(squares.at(0)), destination_indices, static_cast<int>(squares.size() - 1));
        } else {
            return Move(_1_32_to_0_31(squares.at(0)), _1_32_to_0_31(squares.at(1)));
        }
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

    void play_move(Position& position, Move move) noexcept {
        switch (move.type()) {
            case MoveType::Normal:
                assert(position.board[move.source_index()] != Square::None);
                assert(position.board[move.destination_index()] == Square::None);

                std::swap(position.board[move.source_index()], position.board[move.destination_index()]);

                if (!is_king_piece(position.board[move.destination_index()])) {
                    position.plies_without_advancement = 0;
                } else {
                    position.plies_without_advancement++;
                }

                check_piece_crowning(position.board, move.destination_index(), position.player);

                break;
            case MoveType::Capture:
                assert(position.board[move.source_index()] != Square::None);
                assert(
                    position.board[move.destination_index(move.destination_indices_size() - 1)] == Square::None ||
                    move.source_index() == move.destination_index(move.destination_indices_size() - 1)
                );

                remove_jumped_pieces(position.board, move);
                std::swap(
                    position.board[move.source_index()],
                    position.board[move.destination_index(move.destination_indices_size() - 1)]
                );

                position.plies_without_advancement = 0;

                check_piece_crowning(position.board, move.destination_index(move.destination_indices_size() - 1), position.player);

                break;
        }

        position.player = opponent(position.player);
    }

    void play_move(search::SearchNode& node, Move move) noexcept {
        switch (move.type()) {
            case MoveType::Normal:
                assert(node.board[move.source_index()] != Square::None);
                assert(node.board[move.destination_index()] == Square::None);

                std::swap(node.board[move.source_index()], node.board[move.destination_index()]);

                if (!is_king_piece(node.board[move.destination_index()])) {
                    node.plies_without_advancement = 0;
                    node.previous = nullptr;
                } else {
                    node.plies_without_advancement++;
                }

                check_piece_crowning(node.board, move.destination_index(), node.player);

                break;
            case MoveType::Capture:
                assert(node.board[move.source_index()] != Square::None);
                assert(
                    node.board[move.destination_index(move.destination_indices_size() - 1)] == Square::None ||
                    move.source_index() == move.destination_index(move.destination_indices_size() - 1)
                );

                remove_jumped_pieces(node.board, move);
                std::swap(
                    node.board[move.source_index()],
                    node.board[move.destination_index(move.destination_indices_size() - 1)]
                );

                node.plies_without_advancement = 0;
                node.previous = nullptr;

                check_piece_crowning(node.board, move.destination_index(move.destination_indices_size() - 1), node.player);

                break;
        }

        node.player = opponent(node.player);
    }
}
