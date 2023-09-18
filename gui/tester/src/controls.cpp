#include "FL/Fl.H"
#include "FL/Fl_Group.H"
#include "FL/Fl_Button.H"

#include "controls.hpp"

ControlsGroup::ControlsGroup()
    : Fl_Group(500, 0, 300, 400, "Controls") {
    box(FL_BORDER_BOX);

    begin();

    button = new Fl_Button(500, 0, 100, 100, "This is a button. Click me.");

    end();

    resizable(this);
}
