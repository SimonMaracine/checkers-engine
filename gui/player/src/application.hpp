#pragma once

#include <wx/wx.h>

#include "main_window.hpp"

struct Application : public wxApp {
    Application();
    virtual ~Application();

    virtual bool OnInit() override;

    MainWindow* window = nullptr;
};
