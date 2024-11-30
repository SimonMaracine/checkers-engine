#include "evaluation.hpp"

#include "game.hpp"

namespace evaluation {
    Eval static_evaluation(const search::SearchNode& node, const parameters::SearchParameters& parameters) {
        Eval eval {0};

        Eval black_pieces {0};
        Eval black_king_pieces {0};
        Eval white_pieces {0};
        Eval white_king_pieces {0};

        for (game::Idx i {0}; i < 32; i++) {
              switch (node.board[i]) {
                case game::Square::None:
                    break;
                case game::Square::Black:
                    black_pieces++;
                    break;
                case game::Square::BlackKing:
                    black_king_pieces++;
                    break;
                case game::Square::White:
                    white_pieces++;
                    break;
                case game::Square::WhiteKing:
                    white_king_pieces++;
                    break;
            }
        }

        eval -= black_pieces * parameters.piece;
        eval += white_pieces * parameters.piece;
        eval -= black_king_pieces * parameters.piece_king;
        eval += white_king_pieces * parameters.piece_king;

        return eval * perspective(node);
    }
}
