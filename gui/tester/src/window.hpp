#pragma once

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Group.H>

#include "board.hpp"
#include "controls.hpp"

struct MainWindow : public Fl_Window {
    MainWindow();

    Fl_Group* main = nullptr;
    Board* board = nullptr;
    ControlsGroup* controls = nullptr;
};
