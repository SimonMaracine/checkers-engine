#include <forward_list>
#include <list>
#include <functional>
#include <algorithm>

#include <wx/wx.h>

#include "board.hpp"

wxBEGIN_EVENT_TABLE(Board, wxWindow)
    EVT_PAINT(Board::on_paint)
    EVT_MOTION(Board::on_mouse_move)
    EVT_LEFT_DOWN(Board::on_mouse_left_down)
    EVT_RIGHT_DOWN(Board::on_mouse_right_down)
wxEND_EVENT_TABLE()

Board::Board(wxFrame* parent, int x, int y, int size, OnPieceMove on_piece_move)
    : wxWindow(parent, wxID_ANY, wxPoint(x, y), wxSize(size, size)), size(size), on_piece_move(on_piece_move) {
    reset();
}

void Board::set_position(int x, int y) {
    SetPosition(wxPoint(x, y));
}

void Board::set_size(int size) {
    SetSize(size, size);

    this->size = std::clamp(size, 200, 2000);
}

void Board::remove_piece() {

}

void Board::reset() {
    pieces.clear();

    for (int file = 0; file < 8; file++) {
        for (int rank = 0; rank < 3; rank++) {
            if ((file + rank) % 2 == 1) {
                Piece piece;
                piece.square.file = file;
                piece.square.rank = rank;
                piece.color = Piece::Black;

                pieces.push_front(piece);
            }
        }
    }

    for (int file = 0; file < 8; file++) {
        for (int rank = 5; rank < 8; rank++) {
            if ((file + rank) % 2 == 1) {
                Piece piece;
                piece.square.file = file;
                piece.square.rank = rank;
                piece.color = Piece::White;

                pieces.push_front(piece);
            }
        }
    }
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

    const auto square = get_square(event.GetPosition());

    for (Piece& piece : pieces) {
        if (piece.square == square) {
            if (selected_piece != nullptr && piece.square == selected_piece->square) {
                selected_piece = nullptr;
            } else {
                selected_piece = &piece;
            }

            return;
        }
    }

    if (selected_piece == nullptr || !on_piece_move) {
        return;
    }

    if (on_piece_move(selected_piece->square, square, *selected_piece, selected_squares)) {
        selected_piece->square = square;

        selected_piece = nullptr;
        selected_squares.clear();
    }
}

void Board::on_mouse_right_down(wxMouseEvent& event) {
    Refresh();

    const auto square = get_square(event.GetPosition());

    if (selected_piece == nullptr) {
        return;
    }

    for (Square selected_square : selected_squares) {
        if (selected_square == square) {
            selected_squares.remove(square);

            return;
        }
    }

    selected_squares.push_back(square);
}

Board::Square Board::get_square(wxPoint position) {
    const int SQUARE_SIZE = size / 8;

    Square square;
    square.file = position.x / SQUARE_SIZE;
    square.rank = position.y / SQUARE_SIZE;

    return square;
}

void Board::draw(wxDC& dc) {
    const auto SQUARE_WHITE = wxColour(200, 200, 200);
    const auto SQUARE_BLACK = wxColour(80, 60, 40);
    const int SQUARE_SIZE = size / 8;

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

    for (const Piece& piece : pieces) {
        wxColour brush_color;
        wxColour pen_color;

        switch (piece.color) {
            case Piece::White:
                brush_color = PIECE_WHITE;
                pen_color = PIECE_WHITE;
                break;
            case Piece::Black:
                brush_color = PIECE_BLACK;
                pen_color = PIECE_BLACK;
                break;
        }

        if (selected_piece != nullptr) {
            if (selected_piece->square == piece.square) {
                pen_color = wxColour(255, 255, 0);
            }
        }

        dc.SetBrush(wxBrush(brush_color));
        dc.SetPen(wxPen(pen_color));

        const auto position = wxPoint(
            SQUARE_SIZE * piece.square.file + OFFSET, SQUARE_SIZE * piece.square.rank + OFFSET
        );

        dc.DrawCircle(position, PIECE_SIZE);

        if (piece.king) {
            const auto color = wxColour(140, 140, 10);

            dc.SetBrush(wxBrush(color));
            dc.SetPen(wxPen(color));
            dc.DrawCircle(position, PIECE_SIZE / 3);
        }
    }

    for (Square square : selected_squares) {
        dc.SetBrush(wxBrush(wxColour(255, 255, 0)));
        dc.SetPen(wxPen());
        dc.DrawRectangle(wxPoint(SQUARE_SIZE * square.file, SQUARE_SIZE * square.rank), wxSize(SQUARE_SIZE, SQUARE_SIZE));
    }
}
