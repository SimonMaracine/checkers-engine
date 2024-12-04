#include "evaluation.hpp"

#include <algorithm>
#include <cstddef>
#include <cassert>

#include "game.hpp"
#include "search_node.hpp"

namespace evaluation {
    template<typename T>
    static constexpr T map(T x, T in_min, T in_max, T out_min, T out_max) {
        return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
    }

    template<Eval M>
    static constexpr auto positioning_king() {
        return std::array<Eval, 32> {
            M, M, M, M,
            M, 1, 1, 1,
            1, 3, 2, M,
            M, 2, 3, 1,
            1, 3, 2, M,
            M, 2, 3, 1,
            1, 1, 1, M,
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

    static constexpr int count_kings(const search::SearchNode& node) {
        int kings {0};

        for (int i {0}; i < 32; i++) {
            kings += game::is_king_piece(node.board[i]);
        }

        return kings;
    }

    // Bonus points for material (self explanatory)
    static Eval calculate_material(const search::SearchNode& node, const parameters::SearchParameters& parameters) {
        Eval eval {0};

        int black_pawns {0};
        int black_kings {0};
        int white_pawns {0};
        int white_kings {0};

        for (int i {0}; i < 32; i++) {
            switch (node.board[i]) {
                case game::Square::None:
                    break;
                case game::Square::Black:
                    black_pawns++;
                    break;
                case game::Square::BlackKing:
                    black_kings++;
                    break;
                case game::Square::White:
                    white_pawns++;
                    break;
                case game::Square::WhiteKing:
                    white_kings++;
                    break;
            }
        }

        eval -= black_pawns * parameters.material_pawn;
        eval += white_pawns * parameters.material_pawn;
        eval -= black_kings * parameters.material_king;
        eval += white_kings * parameters.material_king;

        return eval;
    }

    // Bonus points are given for pieces that are in better spots
    // Pawn pieces are encouraged to advance the rank while generally avoiding the sides (less freedom)
    // King pieces are encouraged to control the center (this helps the endgame)
    // King pieces' table change depending on how many kings there are in the game
    static Eval calculate_positioning(const search::SearchNode& node, const parameters::SearchParameters& parameters) {
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

        static constexpr auto POSITIONING_KING {positioning_king<-1>()};

        Eval eval {0};

        for (int i {0}; i < 32; i++) {
            switch (node.board[i]) {
                case game::Square::None:
                    break;
                case game::Square::Black:
                    eval -= POSITIONING_PAWN_BLACK[i] * parameters.positioning_pawn;
                    break;
                case game::Square::BlackKing:
                    eval -= POSITIONING_KING[i] * parameters.positioning_king;
                    break;
                case game::Square::White:
                    eval += POSITIONING_PAWN_WHITE[i] * parameters.positioning_pawn;
                    break;
                case game::Square::WhiteKing:
                    eval += POSITIONING_KING[i] * parameters.positioning_king;
                    break;
            }
        }

        return eval;
    }

    // Bonus points are given for pieces that have neighbors of the same color
    static Eval calculate_crowdness(const search::SearchNode& node, const parameters::SearchParameters& parameters) {
        Eval eval {0};

        for (int i {0}; i < 32; i++) {
            switch (node.board[i]) {
                case game::Square::None:
                    break;
                case game::Square::Black:
                case game::Square::BlackKing: {
                    const int count {neighbors<game::Player::Black>(i, node.board)};

                    if (count == 4) {
                        eval -= 2 * parameters.crowdness;
                    } else {
                        eval -= static_cast<Eval>(count) * parameters.crowdness;
                    }

                    break;
                }
                case game::Square::White:
                case game::Square::WhiteKing: {
                    const int count {neighbors<game::Player::White>(i, node.board)};

                    if (count == 4) {
                        eval += 2 * parameters.crowdness;
                    } else {
                        eval += static_cast<Eval>(count) * parameters.crowdness;
                    }

                    break;
                }
            }
        }

        return eval;
    }

    // static Eval calculate_engagement(const search::SearchNode& node, const parameters::SearchParameters& parameters) {
    //     struct Position {
    //         int x {};
    //         int y {};
    //     };

    //     static constexpr Position WORLD[] {
    //         { 1, 0 }, { 3, 0 }, { 5, 0 }, { 7, 0 },
    //         { 0, 1 }, { 2, 1 }, { 4, 1 }, { 6, 1 },
    //         { 1, 2 }, { 3, 2 }, { 5, 2 }, { 7, 2 },
    //         { 0, 3 }, { 2, 3 }, { 4, 3 }, { 6, 3 },
    //         { 1, 4 }, { 3, 4 }, { 5, 4 }, { 7, 4 },
    //         { 0, 5 }, { 2, 5 }, { 4, 5 }, { 6, 5 },
    //         { 1, 6 }, { 3, 6 }, { 5, 6 }, { 7, 6 },
    //         { 0, 7 }, { 2, 7 }, { 4, 7 }, { 6, 7 }
    //     };

    //     Eval eval {0};

    // }

    Eval static_evaluation(const search::SearchNode& node, const parameters::SearchParameters& parameters) {
        Eval eval {0};
        eval += calculate_material(node, parameters);
        eval += calculate_positioning(node, parameters);
        eval += calculate_crowdness(node, parameters);

        return eval;
    }
}
