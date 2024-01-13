#include "game.hpp"

#include <regex>
#include <cstddef>
#include <utility>
#include <stdexcept>
#include <array>
#include <cmath>

#include "moves.hpp"
#include "error.hpp"

namespace game {
    namespace pdn {
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

            return {};
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
                // Here, square is based on the formal indexing of the board: [1, 32]
                const auto [square, king] {parse_piece(fen_string, index)};

                if (player == game::Player::Black) {
                    if (king) {
                        board[square - 1] = game::Square::BlackKing;
                    } else {
                        board[square - 1] = game::Square::Black;
                    }
                } else {
                    if (king) {
                        board[square - 1] = game::Square::WhiteKing;
                    } else {
                        board[square - 1] = game::Square::White;
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
                } else if (move_string[index] == '-' || move_string[index] == 'x') {
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

            return static_cast<game::Idx>(result);
        }

        static game::Idx parse_source_square(const std::string& move_string, std::size_t& index) {
            const auto number {parse_number(move_string, index)};

            if (number > 32u) {
                throw error::ERR;
            }

            return static_cast<game::Idx>(number);
        }

        static std::pair<std::array<game::Idx, 9>, std::size_t> parse_destination_squares(const std::string& move_string, std::size_t& index) {
            std::array<game::Idx, 9> indices {};
            std::size_t count {0u};

            while (true) {
                if (index == move_string.size()) {
                    break;
                }

                const auto number {parse_number(move_string, index)};

                if (number > 32u) {
                    throw error::ERR;
                }

                indices[count++] = static_cast<game::Idx>(number);
            }

            return std::make_pair(indices, count);
        }

        static bool is_capture_move(game::Idx source, const std::array<game::Idx, 9>& destinations, std::size_t count) {
            if (count == 1) {
                const auto distance {std::abs(source - 1 - destinations[0u] - 1)};

                if (distance >= 4 && distance <= 6) {
                    // Then it can't be a capture move
                    return false;
                } else {
                    return true;
                }
            }

            return true;
        }
    }

    void set_position(FenPosition& position, const std::string& fen_string) {
        if (!pdn::valid_fen_string(fen_string)) {
            throw error::ERR;
        }

        std::size_t index {0u};

        position.player = pdn::parse_player(fen_string, index);

        index++;
        index++;

        const game::Player player1 {pdn::parse_player(fen_string, index)};

        index++;

        pdn::parse_pieces(fen_string, index, player1, position.board);

        index++;

        const game::Player player2 {pdn::parse_player(fen_string, index)};

        index++;

        pdn::parse_pieces(fen_string, index, player2, position.board);
    }

    void make_move(Position& position, const std::string& move_string) {
        if (!pdn::valid_move_string(move_string)) {
            throw error::ERR;
        }

        std::size_t index {0u};

        // These are in range [1, 32] and need to be shifted
        const auto source {pdn::parse_source_square(move_string, index)};
        const auto [destinations, count] {pdn::parse_destination_squares(move_string, index)};

        // Construct a move and play it

        game::Move move;

        if (pdn::is_capture_move(source, destinations, count)) {
            move.type = game::MoveType::Capture;
            move.capture.source_index = source - 1;
            move.capture.destination_indices_size = static_cast<unsigned char>(count);

            for (std::size_t i {0u}; i < count; i++) {
                move.capture.destination_indices[i] = destinations[i] - 1;
            }
        } else {
            move.type = game::MoveType::Normal;
            move.normal.source_index = source - 1;
            move.normal.destination_index = destinations[0u] - 1;
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
}
