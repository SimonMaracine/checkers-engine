#pragma once

#include <wx/wx.h>

class MainWindow : public wxFrame {
public:
    MainWindow();
    ~MainWindow();
private:
    void setup_menubar();
    void setup_widgets();

    void on_exit(wxCommandEvent& event);
    void on_about(wxCommandEvent& event);

    wxDECLARE_EVENT_TABLE();
};
