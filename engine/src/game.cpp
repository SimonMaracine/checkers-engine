#include "game.hpp"

#include <regex>
#include <cstddef>
#include <utility>
#include <stdexcept>
#include <cmath>
#include <cassert>

#include "moves.hpp"
#include "error.hpp"
#include "search_node.hpp"

namespace game {
    static bool valid_fen_string(const std::string& fen_string) {
        const std::regex pattern {"(W|B)(:(W|B)K?[0-9]+(,K?[0-9]+){0,11}){2}"};

        return std::regex_match(fen_string, pattern);
    }

    static game::Player parse_player(const std::string& fen_string, std::size_t index) {
        switch (fen_string[index]) {
            case 'B':
                return game::Player::Black;
            case 'W':
                return game::Player::White;
        }

        throw error::ERR;
    }

    static std::pair<game::Idx, bool> parse_piece(const std::string& fen_string, std::size_t& index) {
        bool scanning {true};
        bool king {false};
        std::string result_number;

        while (scanning) {
            if (index == fen_string.size()) {
                // Also stop scanning when going past the string
                break;
            }

            switch (fen_string[index]) {
                case 'K':
                    king = true;
                    index++;

                    break;
                case ',':
                    scanning = false;
                    index++;

                    break;
                case ':':
                    scanning = false;

                    break;
                case '0':
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                case '8':
                case '9':
                    result_number.push_back(fen_string[index]);
                    index++;

                    break;
            }
        }

        unsigned long result {0u};

        try {
            result = std::stoul(result_number);
        } catch (const std::invalid_argument&) {
            throw error::ERR;
        } catch (const std::out_of_range&) {
            throw error::ERR;
        }

        return std::make_pair(static_cast<game::Idx>(result), king);
    }

    static void parse_pieces(const std::string& fen_string, std::size_t& index, game::Player player, game::Board& board) {
        while (fen_string[index] != ':' && index != fen_string.size()) {
            // Squares are in the range [1, 32]
            const auto [square, king] {parse_piece(fen_string, index)};

            if (player == game::Player::Black) {
                if (king) {
                    board[to_0_31(square)] = game::Square::BlackKing;
                } else {
                    board[to_0_31(square)] = game::Square::Black;
                }
            } else {
                if (king) {
                    board[to_0_31(square)] = game::Square::WhiteKing;
                } else {
                    board[to_0_31(square)] = game::Square::White;
                }
            }
        }
    }

    static bool valid_move_string(const std::string& move_string) {
        const std::regex pattern {"([0-9]+x)+[0-9]+"};

        return std::regex_match(move_string, pattern);
    }

    static unsigned int parse_number(const std::string& move_string, std::size_t& index) {
        std::string result_number;

        while (true) {
            if (index == move_string.size()) {
                // Also stop scanning when going past the string
                break;
            }

            if (move_string[index] >= '0' && move_string[index] <= '9') {
                result_number.push_back(move_string[index]);
                index++;
            } else if (move_string[index] == 'x') {
                index++;
                break;
            } else {
                throw error::ERR;
            }
        }

        unsigned long result {0u};

        try {
            result = std::stoul(result_number);
        } catch (const std::invalid_argument&) {
            throw error::ERR;
        } catch (const std::out_of_range&) {
            throw error::ERR;
        }

        return static_cast<unsigned int>(result);
    }

    static game::Idx parse_source_square(const std::string& move_string, std::size_t& index) {
        const auto number {parse_number(move_string, index)};

        if (number < 1u || number > 32u) {
            throw error::ERR;
        }

        return static_cast<game::Idx>(number);
    }

    static std::pair<std::array<game::Idx, 9u>, std::size_t> parse_destination_squares(const std::string& move_string, std::size_t& index) {
        std::array<game::Idx, 9u> indices {};
        std::size_t count {0u};

        while (true) {
            if (index == move_string.size()) {
                break;
            }

            const auto number {parse_number(move_string, index)};

            if (number < 1u || number > 32u) {
                throw error::ERR;
            }

            indices[count++] = static_cast<game::Idx>(number);
        }

        return std::make_pair(indices, count);
    }

    static bool is_capture_move(game::Idx source, game::Idx destination) {
        // Indices must be in the range [1, 32]

        const auto distance {std::abs(to_0_31(source) - to_0_31(destination))};

        if (distance >= 3 && distance <= 5) {
            return false;
        } else if (distance == 7 || distance == 9) {
            return true;
        } else {
            assert(false);
        }
    }

    void set_position(FenPosition& position, const std::string& fen_string) {
        if (!valid_fen_string(fen_string)) {
            throw error::ERR;
        }

        // Clear the board first
        position.board = {};

        std::size_t index {0u};

        position.player = parse_player(fen_string, index);

        index++;
        index++;

        const game::Player player1 {parse_player(fen_string, index)};

        index++;

        parse_pieces(fen_string, index, player1, position.board);

        index++;

        const game::Player player2 {parse_player(fen_string, index)};

        index++;

        parse_pieces(fen_string, index, player2, position.board);
    }

    void make_move(Position& position, const std::string& move_string) {
        if (!valid_move_string(move_string)) {
            throw error::ERR;
        }

        std::size_t index {0u};

        // These are in the range [1, 32]
        const auto source {parse_source_square(move_string, index)};
        const auto [destinations, count] {parse_destination_squares(move_string, index)};

        if (count == 0u) {
            throw error::ERR;
        }

        // Construct a move and play it
        game::Move move;

        if (is_capture_move(source, destinations[0u])) {
            move.type = game::MoveType::Capture;
            move.capture.source_index = to_0_31(source);
            move.capture.destination_indices_size = static_cast<unsigned char>(count);

            for (std::size_t i {0u}; i < count; i++) {
                move.capture.destination_indices[i] = to_0_31(destinations[i]);
            }
        } else {
            move.type = game::MoveType::Normal;
            move.normal.source_index = to_0_31(source);
            move.normal.destination_index = to_0_31(destinations[0u]);
        }

        moves::play_move(position, move);
    }

    Player opponent(Player player) {
        if (player == Player::Black) {  // TODO opt. maybe do xor
            return Player::White;
        } else {
            return Player::Black;
        }
    }

    bool is_game_over(const search::SearchNode& node) {
        unsigned int black_pieces {0u};
        unsigned int white_pieces {0u};

        for (Idx i {0}; i < 32; i++) {
            if (static_cast<unsigned char>(node.board[i]) & 1u << 0) {  // TODO opt.
                black_pieces++;
            } else if (static_cast<unsigned char>(node.board[i]) & 1u << 1) {
                white_pieces++;
            }
        }

        if (black_pieces == 0u || white_pieces == 0u) {
            return true;
        }

        return false;
    }
}
