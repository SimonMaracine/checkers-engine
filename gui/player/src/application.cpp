#include "application.hpp"

wxIMPLEMENT_APP(Application);

bool Application::OnInit() {
    window = new MainWindow;
    window->Show();

    return true;
}
