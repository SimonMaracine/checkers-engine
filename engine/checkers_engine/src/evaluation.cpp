#include "evaluation.hpp"

#include "game.hpp"

namespace evaluation {
    Eval static_evaluation(const search::SearchNode& node, const evaluation::Parameters& parameters) {
        Eval eval {0};

        unsigned int black_pieces {0};
        unsigned int white_pieces {0};

        for (game::Idx i {0}; i < 32; i++) {
            if (static_cast<unsigned char>(node.board[i]) & 1 << 0) {  // TODO opt.
                black_pieces++;
            } else if (static_cast<unsigned char>(node.board[i]) & 1 << 1) {
                white_pieces++;
            }
        }

        eval -= black_pieces * parameters.PIECE;
        eval += white_pieces * parameters.PIECE;

        return eval;
    }
}
