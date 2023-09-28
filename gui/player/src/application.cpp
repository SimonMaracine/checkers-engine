#include <wx/wx.h>

#include "application.hpp"
#include "main_window.hpp"

wxIMPLEMENT_APP(Application);

Application::Application() {

}

Application::~Application() {

}

bool Application::OnInit() {
    window = new MainWindow;
    window->Show();

    return true;
}
