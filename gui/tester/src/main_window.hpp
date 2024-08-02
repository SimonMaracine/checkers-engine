#pragma once

#include <memory>
#include <string>
#include <vector>
#include <utility>
#include <optional>

#include <common/wx.hpp>
#include <common/board.hpp>
#include <common/engine.hpp>
#include <common/parameters.hpp>
#include <common/moves_log.hpp>
#include <common/game_state.hpp>

class MainWindow : public wxFrame {
public:
    MainWindow();
private:
    enum class Player {
        Black,
        White
    };

    void setup_menubar();
    void setup_widgets();

    void on_start_engine_black(wxCommandEvent&);
    void on_start_engine_white(wxCommandEvent&);
    void on_show_indices(wxCommandEvent&);
    void on_exit(wxCommandEvent&);
    void on_close(wxCloseEvent&);
    void on_about(wxCommandEvent&);
    void on_window_resize(wxSizeEvent& event);
    void on_play_position(wxCommandEvent&);
    void on_play_positions(wxCommandEvent&);
    void on_stop(wxCommandEvent&);

    void start_engine(
        std::unique_ptr<engine::Engine>& engine,
        wxStaticText* txt_engine,
        const wxString& text,
        parameters::ParametersPanel* pnl_parameters
    );

    void on_piece_move(const board::CheckersBoard::Move& move);
    void on_engine_message(const std::string& message, bool error, Player player);

    void set_position(const std::optional<std::string>& fen_string);
    int get_ideal_board_size();
    std::vector<std::string> parse_message(const std::string& message);
    void read_positions_file(const std::string& file_path);

    board::CheckersBoard* board {nullptr};

    wxMenuItem* btn_play_position {nullptr};
    wxMenuItem* btn_play_positions {nullptr};
    wxMenuItem* btn_stop {nullptr};

    wxStaticText* txt_engine_black {nullptr};
    wxStaticText* txt_engine_white {nullptr};

    wxStaticText* txt_eval_black {nullptr};
    wxStaticText* txt_eval_white {nullptr};

    std::unique_ptr<engine::Engine> engine_black;
    std::unique_ptr<engine::Engine> engine_white;

    game_state::GameStatePanel* pnl_game_state {nullptr};
    moves_log::MovesLog* pnl_moves_log {nullptr};
    parameters::ParametersPanel* pnl_parameters_black {nullptr};
    parameters::ParametersPanel* pnl_parameters_white {nullptr};

    wxDECLARE_EVENT_TABLE();
};
