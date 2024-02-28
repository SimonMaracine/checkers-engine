#pragma once

#include <memory>
#include <string>
#include <vector>
#include <utility>
#include <optional>

#ifdef __GNUG__
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wconversion"  // Ignore stupid wxWidgets warnings
#endif

#include <wx/wx.h>

#ifdef __GNUG__
    #pragma GCC diagnostic pop
#endif

#include <common/board.hpp>
#include <common/engine.hpp>
#include <common/parameters.hpp>
#include <common/moves_log.hpp>
#include <common/game_state.hpp>

class MainWindow : public wxFrame {
public:
    MainWindow();
private:
    enum class PlayerType {
        Human,
        Computer
    };

    void setup_menubar();
    void setup_widgets();

    void on_start_engine(wxCommandEvent&);
    void on_reset_position(wxCommandEvent&);
    void on_set_position(wxCommandEvent&);
    void on_show_indices(wxCommandEvent&);
    void on_exit(wxCommandEvent&);
    void on_close(wxCloseEvent&);
    void on_about(wxCommandEvent&);
    void on_window_resize(wxSizeEvent& event);
    void on_black_change(wxCommandEvent&);
    void on_white_change(wxCommandEvent&);
    void on_stop(wxCommandEvent&);
    void on_continue(wxCommandEvent&);

    void on_piece_move(const board::CheckersBoard::Move& move);
    void on_engine_message(const std::string& message);

    void set_position(const std::optional<std::string>& fen_string);
    int get_ideal_board_size();
    PlayerType get_player_type(board::CheckersBoard::Player player);
    void process_engine_message(const std::string& message);
    std::vector<std::string> parse_message(const std::string& message);

    board::CheckersBoard* board {nullptr};

    wxRadioButton* btn_black_human {nullptr};
    wxRadioButton* btn_black_computer {nullptr};

    wxRadioButton* btn_white_human {nullptr};
    wxRadioButton* btn_white_computer {nullptr};

    wxButton* btn_stop {nullptr};
    wxButton* btn_continue {nullptr};

    PlayerType black {PlayerType::Human};
    PlayerType white {PlayerType::Computer};

    wxStaticText* txt_engine {nullptr};

    std::unique_ptr<engine::Engine> engine;

    game_state::GameStatePanel* pnl_game_state {nullptr};
    moves_log::MovesLog* pnl_moves_log {nullptr};
    parameters::ParametersPanel* pnl_parameters {nullptr};

    wxDECLARE_EVENT_TABLE();
};
