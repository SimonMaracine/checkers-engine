#include <array>
#include <functional>
#include <utility>
#include <vector>
#include <algorithm>
#include <cstddef>
#include <cmath>

#include <wx/wx.h>

#include "board.hpp"

/*
    TODO
    threefold repetition
    80 moves rule
    finish capture move implementation

    board redraw
*/

wxBEGIN_EVENT_TABLE(Board, wxWindow)
    EVT_PAINT(Board::on_paint)
    EVT_LEFT_DOWN(Board::on_mouse_left_down)
wxEND_EVENT_TABLE()

Board::Board(wxFrame* parent, int x, int y, int size, OnPieceMove on_piece_move)
    : wxWindow(parent, wxID_ANY, wxPoint(x, y), wxSize(size, size)), board_size(size), on_piece_move(on_piece_move) {
    reset();
}

void Board::set_position(int x, int y) {
    SetPosition(wxPoint(x, y));
}

void Board::set_size(int size) {
    SetSize(size, size);

    board_size = std::clamp(size, 200, 2000);
}

void Board::reset() {
    std::fill(std::begin(board), std::end(board), Square::None);
    selected_piece_index = NULL_INDEX;
    turn = Player::Black;

    for (Idx i = 0; i < 24; i++) {
        const auto [file, rank] = get_square(i);

        if ((file + rank) % 2 == 1) {
            board[i] = Square::White;
        }
    }

    for (Idx i = 40; i < 64; i++) {
        const auto [file, rank] = get_square(i);

        if ((file + rank) % 2 == 1) {
            board[i] = Square::Black;
        }
    }

    Refresh();
}

void Board::on_paint(wxPaintEvent& event) {
    wxPaintDC dc {this};
    draw(dc);
}

void Board::on_mouse_left_down(wxMouseEvent& event) {
    Refresh();

    const Idx square_index = get_square(event.GetPosition());

    if (select_piece(square_index)) {
        return;
    }

    if (selected_piece_index == NULL_INDEX) {
        return;
    }

    if (!on_piece_move) {
        return;
    }

    for (const Move& move : legal_moves) {
        switch (move.type) {
            case MoveType::Normal:
                try_play_normal_move(move, square_index);
                break;
            case MoveType::Capture:
                try_play_capture_move(move, square_index);
                break;
        }
    }
}

Board::Idx Board::get_square(wxPoint position) {
    const Idx SQUARE_SIZE = board_size / 8;

    const Idx file = position.x / SQUARE_SIZE;
    const Idx rank = position.y / SQUARE_SIZE;

    return rank * 8 + file;
}

std::pair<Board::Idx, Board::Idx> Board::get_square(Idx square_index) {
    const Idx file = square_index % 8;
    const Idx rank = square_index / 8;

    return std::make_pair(file, rank);
}

bool Board::select_piece(Idx square_index) {
    for (Idx i = 0; i < 64; i++) {
        if (i != square_index) {
            continue;
        }

        if (selected_piece_index == square_index) {
            selected_piece_index = NULL_INDEX;
            legal_moves.clear();

            return false;
        }

        if (board[square_index] != Square::None) {
            selected_piece_index = square_index;
            legal_moves = generate_moves();

            return true;
        }
    }

    return false;
}

std::vector<Board::Move> Board::generate_moves() {
    std::vector<Move> moves;

    for (Idx i = 0; i < 64; i++) {
        const bool king = static_cast<unsigned int>(board[i]) & (1u << 2);
        const bool piece = static_cast<unsigned int>(board[i]) & static_cast<unsigned int>(turn);

        if (piece) {
            generate_piece_capture_moves(moves, i, turn, king);
        }
    }

    // If there are possible captures, force the player to play these moves
    if (!moves.empty()) {
        return moves;
    }

    for (Idx i = 0; i < 64; i++) {
        const bool king = static_cast<unsigned int>(board[i]) & (1u << 2);
        const bool piece = static_cast<unsigned int>(board[i]) & static_cast<unsigned int>(turn);

        if (piece) {
            generate_piece_moves(moves, i, turn, king);
        }
    }

    return moves;
}

void Board::generate_piece_capture_moves(std::vector<Move>& moves, Idx square_index, Player player, bool king) {
    JumpCtx ctx;
    ctx.source_index = square_index;

    check_piece_jumps(moves, square_index, player, king, ctx);
}

void Board::generate_piece_moves(std::vector<Move>& moves, Idx square_index, Player player, bool king) {
    Direction directions[4] {};
    std::size_t index = 0;

    if (king) {
        directions[index++] = Direction::NorthEast;
        directions[index++] = Direction::NorthWest;
        directions[index++] = Direction::SouthEast;
        directions[index++] = Direction::SouthWest;
    } else {
        switch (player) {
            case Player::Black:
                directions[index++] = Direction::NorthEast;
                directions[index++] = Direction::NorthWest;
                break;
            case Player::White:
                directions[index++] = Direction::SouthEast;
                directions[index++] = Direction::SouthWest;
                break;
        }
    }

    // Check the squares above or below in diagonal
    for (std::size_t i = 0; i < index; i++) {
        const Idx target_index = offset(square_index, directions[i], Short);

        if (target_index == NULL_INDEX) {
            continue;
        }

        if (board[target_index] != Square::None) {
            continue;
        }

        Move move;
        move.type = MoveType::Normal;
        move.normal.source_index = square_index;
        move.normal.destination_index = target_index;

        moves.push_back(move);
    }
}

bool Board::check_piece_jumps(std::vector<Move>& moves, Idx square_index, Player player, bool king, JumpCtx& ctx) {
    Direction directions[4] {};
    std::size_t index = 0;

    if (king) {
        directions[index++] = Direction::NorthEast;
        directions[index++] = Direction::NorthWest;
        directions[index++] = Direction::SouthEast;
        directions[index++] = Direction::SouthWest;
    } else {
        switch (player) {
            case Player::Black:
                directions[index++] = Direction::NorthEast;
                directions[index++] = Direction::NorthWest;
                break;
            case Player::White:
                directions[index++] = Direction::SouthEast;
                directions[index++] = Direction::SouthWest;
                break;
        }
    }

    // We want an enemy piece
    unsigned int piece_mask;

    switch (player) {
        case Player::Black:
            piece_mask = 0b0010u;
            break;
        case Player::White:
            piece_mask = 0b0001u;
            break;
    }

    for (std::size_t i = 0; i < index; i++) {
        const Idx enemy_index = offset(square_index, directions[i], Short);
        const Idx target_index = offset(square_index, directions[i], Long);

        if (enemy_index == NULL_INDEX || target_index == NULL_INDEX) {
            continue;
        }

        const bool is_enemy_piece = static_cast<unsigned int>(board[enemy_index]) & piece_mask;

        if (!is_enemy_piece || board[target_index] != Square::None) {
            continue;
        }

        ctx.intermediary_square_indices.push_back(target_index);
        ctx.captured_pieces_indices.push_back(enemy_index);

        // Temporarily remove the piece to avoid illegal jumps
        const Square enemy_piece = board[enemy_index];
        board[enemy_index] = Square::None;

        if (!check_piece_jumps(moves, target_index, player, king, ctx)) {
            // This means that it reached the end of a sequence of jumps

            Move move;
            move.type = MoveType::Capture;
            move.capture.source_index = ctx.source_index;
            move.capture.destination_index = target_index;
            move.capture.intermediary_square_indices_size = ctx.intermediary_square_indices.size();
            move.capture.captured_pieces_indices_size = ctx.captured_pieces_indices.size();

            for (std::size_t i = 0; i < ctx.captured_pieces_indices.size(); i++) {
                move.capture.intermediary_square_indices[i] = ctx.intermediary_square_indices[i];
                move.capture.captured_pieces_indices[i] = ctx.captured_pieces_indices[i];
            }

            moves.push_back(move);
        }

        // Restore removed piece
        board[enemy_index] = enemy_piece;

        ctx.intermediary_square_indices.pop_back();
        ctx.captured_pieces_indices.pop_back();

        return true;
    }

    return false;
}

Board::Idx Board::offset(Idx square_index, Direction direction, Diagonal diagonal) {
    static constexpr int OFFSET[2] = { 1, 2 };

    Idx result_index = square_index;

    switch (direction) {
        case Direction::NorthEast:
            result_index -= 7 * OFFSET[diagonal];
            break;
        case Direction::NorthWest:
            result_index -= 9 * OFFSET[diagonal];
            break;
        case Direction::SouthEast:
            result_index += 9 * OFFSET[diagonal];
            break;
        case Direction::SouthWest:
            result_index += 7 * OFFSET[diagonal];
            break;
    }

    // Check edge cases (literally)
    if (std::abs(square_index / 8 - result_index / 8) != OFFSET[diagonal]) {
        return NULL_INDEX;
    }

    // Check out of bounds
    if (result_index < 0 || result_index > 63) {
        return NULL_INDEX;
    }

    return result_index;
}

void Board::change_turn() {
    if (turn == Player::Black) {
        turn = Player::White;
    } else {
        turn = Player::Black;
    }
}

void Board::check_piece_crowning(Idx square_index, Player player) {
    const Idx index = square_index / 8;

    switch (turn) {
        case Player::Black:
            if (index == 0) {
                board[square_index] = Square::BlackKing;
            }

            break;
        case Player::White:
            if (index == 7) {
                board[square_index] = Square::WhiteKing;
            }

            break;
    }
}

void Board::try_play_normal_move(const Move& move, Idx square_index) {
    if (move.normal.source_index != selected_piece_index || move.normal.destination_index != square_index) {
        return;
    }

    if (on_piece_move(move)) {
        std::swap(board[move.normal.source_index], board[move.normal.destination_index]);

        check_piece_crowning(move.normal.destination_index, turn);

        selected_piece_index = NULL_INDEX;
        change_turn();
    }
}

void Board::try_play_capture_move(const Move& move, Idx square_index) {
    if (move.capture.source_index != selected_piece_index) {
        return;
    }

    if (move.capture.intermediary_square_indices[0] != square_index) {  // FIXME
        return;
    }

    if (on_piece_move(move)) {
        std::swap(board[move.capture.source_index], board[move.capture.destination_index]);

        for (Idx i = 0; i < move.capture.captured_pieces_indices_size; i++) {
            board[move.capture.captured_pieces_indices[i]] = Square::None;
        }

        check_piece_crowning(move.capture.destination_index, turn);

        selected_piece_index = NULL_INDEX;
        change_turn();
    }
}

void Board::draw(wxDC& dc) {
    const auto ORANGE = wxColour(240, 180, 80);
    const auto GOLD = wxColour(160, 160, 10);
    const auto WHITE = wxColour(200, 200, 200);
    const auto BLACK = wxColour(80, 60, 40);
    const int SQUARE_SIZE = board_size / 8;

    for (int x = 0; x < 8; x++) {
        for (int y = 0; y < 8; y++) {
            if ((x + y) % 2 == 0) {
                dc.SetBrush(wxBrush(WHITE));
                dc.SetPen(wxPen(WHITE));
            } else {
                dc.SetBrush(wxBrush(BLACK));
                dc.SetPen(wxPen(BLACK));
            }

            dc.DrawRectangle(wxPoint(SQUARE_SIZE * x, SQUARE_SIZE * y), wxSize(SQUARE_SIZE, SQUARE_SIZE));
        }
    }

    if (selected_piece_index != NULL_INDEX) {
        dc.SetBrush(wxBrush(ORANGE));
        dc.SetPen(wxPen(ORANGE));

        for (const Move& move : legal_moves) {
            switch (move.type) {
                case MoveType::Normal: {
                    if (move.normal.source_index == selected_piece_index) {
                        const auto [x, y] = get_square(move.normal.destination_index);

                        dc.DrawRectangle(wxPoint(SQUARE_SIZE * x, SQUARE_SIZE * y), wxSize(SQUARE_SIZE, SQUARE_SIZE));
                    }

                    break;
                }
                case MoveType::Capture: {
                    if (move.capture.source_index == selected_piece_index) {
                        for (Idx i = 0; i < move.capture.intermediary_square_indices_size; i++) {
                            const auto [x, y] = get_square(move.capture.intermediary_square_indices[i]);

                            dc.DrawRectangle(wxPoint(SQUARE_SIZE * x, SQUARE_SIZE * y), wxSize(SQUARE_SIZE, SQUARE_SIZE));
                        }
                    }

                    break;
                }
            }
        }
    }

    const auto PIECE_WHITE = wxColour(140, 10, 10);
    const auto PIECE_BLACK = wxColour(10, 10, 10);
    const int PIECE_SIZE = static_cast<int>(static_cast<float>(SQUARE_SIZE) / 2.5f);
    const int OFFSET = SQUARE_SIZE / 2;

    for (Idx i = 0; i < 64; i++) {
        wxColour brush_color;
        wxColour pen_color;

        bool king = false;

        switch (board[i]) {
            case Square::None:
                continue;
            case Square::White:
                brush_color = PIECE_WHITE;
                pen_color = PIECE_WHITE;
                break;
            case Square::Black:
                brush_color = PIECE_BLACK;
                pen_color = PIECE_BLACK;
                break;
            case Square::WhiteKing:
                brush_color = PIECE_WHITE;
                pen_color = PIECE_WHITE;
                king = true;
                break;
            case Square::BlackKing:
                brush_color = PIECE_BLACK;
                pen_color = PIECE_BLACK;
                king = true;
                break;
        }

        if (selected_piece_index == i) {
            pen_color = ORANGE;
        }

        dc.SetBrush(wxBrush(brush_color));
        dc.SetPen(wxPen(pen_color));

        const auto [x, y] = get_square(i);
        const auto position = wxPoint(SQUARE_SIZE * x + OFFSET, SQUARE_SIZE * y + OFFSET);

        dc.DrawCircle(position, PIECE_SIZE);

        if (king) {
            dc.SetBrush(wxBrush(GOLD));
            dc.SetPen(wxPen(GOLD));
            dc.DrawCircle(position, PIECE_SIZE / 3);
        }
    }
}
