#include <array>
#include <functional>
#include <optional>
#include <utility>
#include <vector>
#include <algorithm>
#include <cstddef>

#include <wx/wx.h>

#include "board.hpp"

wxBEGIN_EVENT_TABLE(Board, wxWindow)
    EVT_PAINT(Board::on_paint)
    EVT_MOTION(Board::on_mouse_move)
    EVT_LEFT_DOWN(Board::on_mouse_left_down)
    EVT_RIGHT_DOWN(Board::on_mouse_right_down)
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

    for (std::size_t i = 0; i < 24; i++) {
        const auto [file, rank] = get_square(i);

        if ((file + rank) % 2 == 1) {
            board[i] = Square::White;
        }
    }

    for (std::size_t i = 40; i < 64; i++) {
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

void Board::on_mouse_move(wxMouseEvent& event) {
    Refresh();
}

void Board::on_mouse_left_down(wxMouseEvent& event) {
    Refresh();

    const int square_index = get_square(event.GetPosition());

    if (select_piece(square_index)) {
        return;
    }

    if (selected_piece_index == NULL_INDEX) {
        return;
    }

    const auto moves = generate_moves();  // TODO don't generate every time

    if (!on_piece_move) {
        return;
    }

    for (Move move : moves) {
        if (move.source_index != selected_piece_index || move.destination_index != square_index) {
            continue;
        }

        if (on_piece_move(move)) {
            std::swap(board[move.source_index], board[move.destination_index]);

            selected_piece_index = NULL_INDEX;
            change_turn();
        }
    }
}

void Board::on_mouse_right_down(wxMouseEvent& event) {
    Refresh();

    const auto square = get_square(event.GetPosition());

    // TODO
}

int Board::get_square(wxPoint position) {
    const int SQUARE_SIZE = board_size / 8;

    const int file = position.x / SQUARE_SIZE;
    const int rank = position.y / SQUARE_SIZE;

    return rank * 8 + file;
}

std::pair<int, int> Board::get_square(int square_index) {
    const int file = square_index % 8;
    const int rank = square_index / 8;

    return std::make_pair(file, rank);
}

bool Board::select_piece(int square_index) {
    for (std::size_t i = 0; i < 64; i++) {
        if (i != square_index) {
            continue;
        }

        if (selected_piece_index == square_index) {
            selected_piece_index = NULL_INDEX;
            return false;
        }

        if (board[square_index] != Square::None) {
            selected_piece_index = square_index;
            return true;
        }
    }

    return false;
}

std::vector<Board::Move> Board::generate_moves() {
    std::vector<Move> moves;

    for (std::size_t i = 0; i < 64; i++) {
        const bool king = static_cast<unsigned int>(board[i]) & (1u << 2);

        if (turn == Player::Black) {
            switch (board[i]) {
                case Square::Black:
                case Square::BlackKing:
                    generate_piece_moves(moves, i, Player::Black, king);
                    break;
                default:
                    break;
            }
        } else {
            switch (board[i]) {
                case Square::White:
                case Square::WhiteKing:
                    generate_piece_moves(moves, i, Player::White, king);
                    break;
                default:
                    break;
            }
        }
    }

    return moves;
}

void Board::generate_piece_moves(std::vector<Move>& moves, int square_index, Player player, bool king) {
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
        const int target_index = offset(square_index, directions[i]);

        if (target_index == NULL_INDEX) {
            continue;
        }

        if (board[target_index] != Square::None) {
            continue;
        }

        Move move;
        move.source_index = square_index;
        move.destination_index = target_index;

        moves.push_back(move);
    }

    // TODO check jumps recursively
}

int Board::offset(int square_index, Direction direction) {
    int result_index = square_index;

    switch (direction) {
        case Direction::NorthEast:
            result_index -= 7;
            break;
        case Direction::NorthWest:
            result_index -= 9;
            break;
        case Direction::SouthEast:
            result_index += 9;
            break;
        case Direction::SouthWest:
            result_index += 7;
            break;
    }

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

void Board::draw(wxDC& dc) {
    const auto SQUARE_WHITE = wxColour(200, 200, 200);
    const auto SQUARE_BLACK = wxColour(80, 60, 40);
    const int SQUARE_SIZE = board_size / 8;

    for (int x = 0; x < 8; x++) {
        for (int y = 0; y < 8; y++) {
            if ((x + y) % 2 == 0) {
                dc.SetBrush(wxBrush(SQUARE_WHITE));
                dc.SetPen(wxPen(SQUARE_WHITE));
            } else {
                dc.SetBrush(wxBrush(SQUARE_BLACK));
                dc.SetPen(wxPen(SQUARE_BLACK));
            }

            dc.DrawRectangle(wxPoint(SQUARE_SIZE * x, SQUARE_SIZE * y), wxSize(SQUARE_SIZE, SQUARE_SIZE));
        }
    }

    const auto PIECE_WHITE = wxColour(140, 10, 10);
    const auto PIECE_BLACK = wxColour(10, 10, 10);
    const int PIECE_SIZE = static_cast<int>(static_cast<float>(SQUARE_SIZE) / 2.5f);
    const int OFFSET = SQUARE_SIZE / 2;

    for (std::size_t i = 0; i < 64; i++) {
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
            pen_color = wxColour(255, 255, 0);
        }

        dc.SetBrush(wxBrush(brush_color));
        dc.SetPen(wxPen(pen_color));

        const auto [file, rank] = get_square(i);
        const auto position = wxPoint(SQUARE_SIZE * file + OFFSET, SQUARE_SIZE * rank + OFFSET);

        dc.DrawCircle(position, PIECE_SIZE);

        if (king) {
            const auto color = wxColour(140, 140, 10);

            dc.SetBrush(wxBrush(color));
            dc.SetPen(wxPen(color));
            dc.DrawCircle(position, PIECE_SIZE / 3);
        }
    }

    // for (Square square : selected_squares) {
    //     dc.SetBrush(wxBrush(wxColour(255, 255, 0)));
    //     dc.SetPen(wxPen());
    //     dc.DrawRectangle(wxPoint(SQUARE_SIZE * square.file, SQUARE_SIZE * square.rank), wxSize(SQUARE_SIZE, SQUARE_SIZE));
    // }
}
