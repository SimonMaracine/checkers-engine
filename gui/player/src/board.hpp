#pragma once

#include <forward_list>
#include <list>
#include <functional>

#include <wx/wx.h>

class Board : public wxWindow {
public:
    struct Square {
        int file {};
        int rank {};

        bool operator==(const Square& other) const {
            return file == other.file && rank == other.rank;
        }
    };

    struct Piece {
        enum {
            White,
            Black
        } color {};

        Square square;

        bool king = false;
    };

    using OnPieceMove = std::function<bool(Square, Square, const Piece&, const std::list<Square>&)>;

    Board(wxFrame* parent, int x, int y, int size, OnPieceMove on_piece_move);

    void set_position(int x, int y);
    void set_size(int size);

    void remove_piece();
    void reset();
private:
    void on_paint(wxPaintEvent& event);
    void on_mouse_move(wxMouseEvent& event);
    void on_mouse_left_down(wxMouseEvent& event);
    void on_mouse_right_down(wxMouseEvent& event);

    Square get_square(wxPoint position);
    void draw(wxDC& dc);

    int size = 0;

    std::forward_list<Piece> pieces;
    Piece* selected_piece = nullptr;
    std::list<Square> selected_squares;

    OnPieceMove on_piece_move;

    wxDECLARE_EVENT_TABLE();
};
