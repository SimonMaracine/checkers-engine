#pragma once

#include <wx/wx.h>

#include "board.hpp"

class MainWindow : public wxFrame {
public:
    MainWindow();
private:
    void setup_menubar();
    void setup_widgets();

    void on_reset_board(wxCommandEvent& event);
    void on_set_position(wxCommandEvent& event);
    void on_exit(wxCommandEvent& event);
    void on_about(wxCommandEvent& event);
    void on_window_resize(wxSizeEvent& event);
    void on_black_change(wxCommandEvent& event);
    void on_white_change(wxCommandEvent& event);

    void on_piece_move(const CheckersBoard::Move& move);

    int get_ideal_board_size();
    const char* game_over_text();

    CheckersBoard* board {nullptr};

    struct {
        wxStaticText* txt_status {nullptr};
        wxStaticText* txt_player {nullptr};
        wxStaticText* txt_plies_without_advancement {nullptr};
        wxStaticText* txt_repetition_size {nullptr};
    } game;

    wxRadioButton* btn_black_human {nullptr};
    wxRadioButton* btn_black_computer {nullptr};

    wxRadioButton* btn_white_human {nullptr};
    wxRadioButton* btn_white_computer {nullptr};

    wxDECLARE_EVENT_TABLE();
};
