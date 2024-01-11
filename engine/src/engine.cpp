#include "engine.hpp"

#include <regex>
#include <cstddef>
#include <utility>
#include <stdexcept>

namespace engine {
    namespace fen {
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
                return std::make_pair(game::NULL_INDEX, false);
            } catch (const std::out_of_range&) {
                return std::make_pair(game::NULL_INDEX, false);
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
    }

    bool set_position(EngineData& data, const std::string& fen_string) {
        if (!fen::valid_fen_string(fen_string)) {
            return false;
        }

        std::size_t index {0u};

        data.position.player = fen::parse_player(fen_string, index);

        index++;
        index++;

        const game::Player player1 {fen::parse_player(fen_string, index)};

        index++;

        fen::parse_pieces(fen_string, index, player1, data.position.board);

        index++;

        const game::Player player2 {fen::parse_player(fen_string, index)};

        index++;

        fen::parse_pieces(fen_string, index, player2, data.position.board);

        return true;
    }

    void reset_position(EngineData& data) {
        set_position(data, "B:B1,2,3,4,5,6,7,8,9,10,11,12:W21,22,23,24,25,26,27,28,29,30,31,32");
    }
}
