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

    void on_reset_board(wxCommandEvent& event);
    void on_set_position(wxCommandEvent& event);
    void on_exit(wxCommandEvent& event);
    void on_about(wxCommandEvent& event);
    void on_window_resize(wxSizeEvent& event);

    void on_piece_move(const Board::Move& move);

    int get_ideal_board_size();
    const char* game_over_text();

    Board* board {nullptr};
    wxPanel* right_side {nullptr};

    struct {
        wxStaticText* status {nullptr};
        wxStaticText* player {nullptr};
        wxStaticText* plies_without_advancement {nullptr};
    } game;

    wxDECLARE_EVENT_TABLE();
};
