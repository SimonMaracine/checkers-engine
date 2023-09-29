#include <iostream>
#include <algorithm>

#include <wx/wx.h>

#include "board.hpp"

wxBEGIN_EVENT_TABLE(Board, wxWindow)
    EVT_PAINT(Board::on_paint)
    EVT_MOTION(Board::on_mouse_move)
    EVT_LEFT_DOWN(Board::on_mouse_down)
    EVT_LEFT_UP(Board::on_mouse_up)
wxEND_EVENT_TABLE()

Board::Board(wxFrame* parent, int x, int y, int size)
    : wxWindow(parent, wxID_ANY, wxPoint(x, y), wxSize(size, size)), size(size) {
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
                piece.file = file;
                piece.rank = rank;
                piece.color = Piece::Black;

                pieces.push_back(piece);
            }
        }
    }

    for (int file = 0; file < 8; file++) {
        for (int rank = 5; rank < 8; rank++) {
            if ((file + rank) % 2 == 1) {
                Piece piece;
                piece.file = file;
                piece.rank = rank;
                piece.color = Piece::White;

                pieces.push_back(piece);
            }
        }
    }
}

void Board::on_paint(wxPaintEvent& event) {
    wxPaintDC dc {this};
    draw(dc);
}

void Board::on_mouse_move(wxMouseEvent& event) {

}

void Board::on_mouse_down(wxMouseEvent& event) {
    std::cout << "Mouse down\n";
}

void Board::on_mouse_up(wxMouseEvent& event) {

}

void Board::draw(wxDC& dc) {
    const auto SQUARE_WHITE = wxColour(200, 200, 200);
    const auto SQUARE_BLACK = wxColour(60, 40, 20);
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
        switch (piece.color) {
            case Piece::White:
                dc.SetBrush(wxBrush(PIECE_WHITE));
                dc.SetPen(wxPen(PIECE_WHITE));
                break;
            case Piece::Black:
                dc.SetBrush(wxBrush(PIECE_BLACK));
                dc.SetPen(wxPen(PIECE_BLACK));
                break;
        }

        const auto position = wxPoint(SQUARE_SIZE * piece.file + OFFSET, SQUARE_SIZE * piece.rank + OFFSET);

        dc.DrawCircle(position, PIECE_SIZE);

        if (piece.king) {
            const auto color = wxColour(140, 140, 10);

            dc.SetBrush(wxBrush(color));
            dc.SetPen(wxPen(color));
            dc.DrawCircle(position, PIECE_SIZE / 3);
        }
    }
}
