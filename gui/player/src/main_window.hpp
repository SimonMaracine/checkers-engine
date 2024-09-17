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
    void on_engine_message(const std::string& message, bool error);

    void stop_engine();
    void initialize_engine();
    void set_position(const std::optional<std::string>& fen_string);
    int get_ideal_board_size();
    PlayerType get_player_type(board::CheckersBoard::Player player);
    std::vector<std::string> parse_message(const std::string& message);

    board::CheckersBoard* m_board {nullptr};

    wxRadioButton* m_btn_black_human {nullptr};
    wxRadioButton* m_btn_black_computer {nullptr};

    wxRadioButton* m_btn_white_human {nullptr};
    wxRadioButton* m_btn_white_computer {nullptr};

    wxButton* m_btn_stop {nullptr};
    wxButton* m_btn_continue {nullptr};

    PlayerType m_black {PlayerType::Human};
    PlayerType m_white {PlayerType::Computer};

    wxStaticText* m_txt_engine {nullptr};

    std::unique_ptr<engine::Engine> m_engine;

    game_state::GameStatePanel* m_pnl_game_state {nullptr};
    moves_log::MovesLog* m_pnl_moves_log {nullptr};
    parameters::ParametersPanel* m_pnl_parameters {nullptr};

    wxDECLARE_EVENT_TABLE();
};
