#pragma once

#ifdef __GNUG__
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wconversion"  // Ignore stupid wxWidgets warnings
#endif

#include <wx/wx.h>

#ifdef __GNUG__
    #pragma GCC diagnostic pop
#endif

#include "main_window.hpp"

struct Application : public wxApp {
    bool OnInit() override;

    MainWindow* window {nullptr};
};
