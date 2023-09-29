#pragma once

#include <vector>

#include <wx/wx.h>

class Board : public wxWindow {
public:
    Board(wxFrame* parent, int x, int y, int size);

    void set_position(int x, int y);
    void set_size(int size);

    void remove_piece();
    void reset();
private:
    void on_paint(wxPaintEvent& event);
    void on_mouse_move(wxMouseEvent& event);
    void on_mouse_down(wxMouseEvent& event);
    void on_mouse_up(wxMouseEvent& event);

    void draw(wxDC& dc);

    int size = 0;

    struct Piece {
        enum {
            White,
            Black
        } color {};

        int file = 0;
        int rank = 0;

        bool king = false;
    };

    std::vector<Piece> pieces;

    wxDECLARE_EVENT_TABLE();
};
