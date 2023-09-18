#include <FL/Fl.H>
#include <FL/Fl_Box.H>
#include <FL/fl_draw.H>

#include "board.hpp"

Board::Board()
    : Fl_Box(0, 0, 500, 500, nullptr) {
    box(FL_BORDER_BOX);
}

void Board::draw() {
    Fl_Box::draw();

    fl_rectf(x, y, width, height, FL_DARK_RED);



    fl_rectf(100, 100, 100, 100, 0xEEEEEE);
}
