#include <iostream>

#include <common/wx.hpp>

#include "main_window.hpp"

struct Application : public wxApp {
    bool OnInit() override;
    bool OnExceptionInMainLoop() override;

    MainWindow* window {nullptr};
};

bool Application::OnInit() {
    window = new MainWindow;
    window->Show();

    return true;
}

bool Application::OnExceptionInMainLoop() {
    std::cerr << "An exception occurred inside the main loop\n";
    wxExit();
}

wxIMPLEMENT_APP(Application);
