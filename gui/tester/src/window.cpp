#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Group.H>

#include "board.hpp"
#include "controls.hpp"
#include "window.hpp"

MainWindow::MainWindow()
    : Fl_Window(1024, 576, "Tester") {
    begin();

    main = new Fl_Group(0, 0, 1024, 576);
    main->begin();

    board = new Board;
    controls = new ControlsGroup;

    main->end();
    end();

    resizable(main);
}
