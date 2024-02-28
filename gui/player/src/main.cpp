#include <common/wx.hpp>

#include "main_window.hpp"

struct Application : public wxApp {
    bool OnInit() override;

    MainWindow* window {nullptr};
};

bool Application::OnInit() {
    window = new MainWindow;
    window->Show();

    return true;
}

wxIMPLEMENT_APP(Application);
