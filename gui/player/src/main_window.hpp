#pragma once

#include <wx/wx.h>

#include "board.hpp"

class MainWindow : public wxFrame {
public:
    MainWindow();
private:
    void setup_menubar();
    void setup_widgets();

    void on_reset_board(wxCommandEvent&);
    void on_set_position(wxCommandEvent&);
    void on_exit(wxCommandEvent&);
    void on_about(wxCommandEvent&);
    void on_window_resize(wxSizeEvent& event);
    void on_black_change(wxCommandEvent&);
    void on_white_change(wxCommandEvent&);

    void on_piece_move(const CheckersBoard::Move& move);

    int get_ideal_board_size();
    const char* game_over_text();
    void update_board_user_input();

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

    enum class Player {
        Human,
        Computer
    };

    Player black {Player::Human};
    Player white {Player::Computer};

    wxDECLARE_EVENT_TABLE();
};
