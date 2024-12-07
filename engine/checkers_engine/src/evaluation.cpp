#include "evaluation.hpp"

#include <algorithm>

#include "game.hpp"
#include "search_node.hpp"

namespace evaluation {
    template<Eval M>
    static constexpr auto positioning_king() {
        return std::array<Eval, 32> {
            M, M, M, M,
            M, 0, 0, 0,
            0, 1, 1, M,
            M, 1, 1, 0,
            0, 1, 1, M,
            M, 1, 1, 0,
            0, 0, 0, M,
            M, M, M, M
        };
    }

    template<game::Player Player, int Offset1, int Offset2>
    static int neighbor(int index, const game::Board& board, bool even_row) {
        const int result_index {index - (even_row ? Offset1 : Offset2)};

        if (std::abs(index / 4 - result_index / 4) != 1) {
            return 0;
        }

        if (result_index < 0 || result_index > 31) {
            return 0;
        }

        return game::is_piece(board[result_index], Player);
    }

    template<game::Player Player>
    static int neighbors(int index, const game::Board& board) {
        const bool even_row {(index / 4) % 2 == 0};
        int count {0};

        count += neighbor<Player, 3, 4>(index, board, even_row);
        count += neighbor<Player, 4, 5>(index, board, even_row);
        count += neighbor<Player, 5, 4>(index, board, even_row);
        count += neighbor<Player, 4, 3>(index, board, even_row);

        return count;
    }

    // Bonus points for material (self explanatory)

    // Bonus points are given for pieces that are in better spots
    // Pawn pieces are encouraged to advance the rank while generally avoiding the sides (less freedom)
    // King pieces are encouraged to control the center (this helps the endgame)

    // Bonus points are given for pieces that have neighbors of the same color

    Eval static_evaluation(const search::SearchNode& node, const parameters::SearchParameters& parameters) {
        static constexpr Eval POSITIONING_PAWN_BLACK[] {
            8, 0, 8, 0,
            0, 1, 2, 1,
            2, 3, 2, 1,
            2, 3, 4, 3,
            4, 5, 4, 3,
            4, 5, 6, 5,
            6, 7, 6, 5,
            0, 0, 0, 0
        };

        static constexpr Eval POSITIONING_PAWN_WHITE[] {
            0, 0, 0, 0,
            5, 6, 7, 6,
            5, 6, 5, 4,
            3, 4, 5, 4,
            3, 4, 3, 2,
            1, 2, 3, 2,
            1, 2, 1, 0,
            0, 8, 0, 8
        };

        static constexpr auto POSITIONING_KING {positioning_king<-2>()};

        Eval eval {0};

        int black_pawns {0};
        int black_kings {0};
        int white_pawns {0};
        int white_kings {0};

        for (int i {0}; i < 32; i++) {
            switch (node.board[i]) {
                case game::Square::None:
                    break;
                case game::Square::Black: {
                    black_pawns++;
                    eval -= POSITIONING_PAWN_BLACK[i] * parameters.positioning_pawn;

                    const int count {neighbors<game::Player::Black>(i, node.board)};

                    if (count == 4) {
                        eval -= 2 * parameters.crowdness;
                    } else {
                        eval -= count * parameters.crowdness;
                    }

                    break;
                }
                case game::Square::BlackKing: {
                    black_kings++;
                    eval -= POSITIONING_KING[i] * parameters.positioning_king;

                    const int count {neighbors<game::Player::Black>(i, node.board)};

                    if (count == 4) {
                        eval -= 2 * parameters.crowdness;
                    } else {
                        eval -= count * parameters.crowdness;
                    }

                    break;
                }
                case game::Square::White: {
                    white_pawns++;
                    eval += POSITIONING_PAWN_WHITE[i] * parameters.positioning_pawn;

                    const int count {neighbors<game::Player::White>(i, node.board)};

                    if (count == 4) {
                        eval += 2 * parameters.crowdness;
                    } else {
                        eval += count * parameters.crowdness;
                    }

                    break;
                }
                case game::Square::WhiteKing: {
                    white_kings++;
                    eval += POSITIONING_KING[i] * parameters.positioning_king;

                    const int count {neighbors<game::Player::White>(i, node.board)};

                    if (count == 4) {
                        eval += 2 * parameters.crowdness;
                    } else {
                        eval += count * parameters.crowdness;
                    }

                    break;
                }
            }
        }

        eval -= black_pawns * parameters.material_pawn;
        eval += white_pawns * parameters.material_pawn;
        eval -= black_kings * parameters.material_king;
        eval += white_kings * parameters.material_king;

        return eval;
    }
}
