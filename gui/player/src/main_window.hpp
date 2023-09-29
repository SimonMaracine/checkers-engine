#pragma once

#include <wx/wx.h>

#include "board.hpp"

class MainWindow : public wxFrame {
public:
    MainWindow();
    ~MainWindow();
private:
    void setup_menubar();
    void setup_widgets();

    void on_exit(wxCommandEvent& event);
    void on_about(wxCommandEvent& event);
    void on_window_resize(wxSizeEvent& event);

    Board* board = nullptr;

    wxDECLARE_EVENT_TABLE();
};
