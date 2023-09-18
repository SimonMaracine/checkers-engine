#pragma once

#include "FL/Fl.H"
#include "FL/Fl_Group.H"
#include "FL/Fl_Button.H"

struct ControlsGroup : public Fl_Group {
    ControlsGroup();

    Fl_Button* button = nullptr;
};
