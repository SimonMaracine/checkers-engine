#pragma once

#include <FL/Fl.H>
#include <FL/Fl_Box.H>

struct Board : public Fl_Box {
    Board();

    virtual void draw() override;

    int x = 0;
    int y = 0;
    int width = 500;
    int height = 500;
};
