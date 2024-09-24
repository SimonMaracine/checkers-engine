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

    // void start_engine(
    //     const std::unique_ptr<engine::Engine>& engine,
    //     wxStaticText* txt_engine,
    //     const wxString& text,
    //     parameters::ParametersPanel* pnl_parameters
    // );

    void on_piece_move(const board::CheckersBoard::Move& move);
    void on_engine_message(const std::string& message, bool error, Player player);

    void start_engine(const std::unique_ptr<engine::Engine>& engine);
    void stop_engine(const std::unique_ptr<engine::Engine>& engine);
    void initialize_engine(
        const std::unique_ptr<engine::Engine>& engine,
        wxStaticText* txt_engine,
        const wxString& text,
        parameters::ParametersPanel* pnl_parameters
    );
    void set_position(const std::optional<std::string>& fen_string);
    int get_ideal_board_size();
    std::vector<std::string> parse_message(const std::string& message);
    void read_positions_file(const std::string& file_path);

    board::CheckersBoard* m_board {nullptr};

    wxMenuItem* m_btn_play_position {nullptr};
    wxMenuItem* m_btn_play_positions {nullptr};
    wxMenuItem* m_btn_stop {nullptr};

    wxStaticText* m_txt_engine_black {nullptr};
    wxStaticText* m_txt_engine_white {nullptr};

    wxStaticText* m_txt_eval_black {nullptr};
    wxStaticText* m_txt_eval_white {nullptr};

    std::unique_ptr<engine::Engine> m_engine_black;
    std::unique_ptr<engine::Engine> m_engine_white;

    game_state::GameStatePanel* m_pnl_game_state {nullptr};
    moves_log::MovesLog* m_pnl_moves_log {nullptr};
    parameters::ParametersPanel* m_pnl_parameters_black {nullptr};
    parameters::ParametersPanel* m_pnl_parameters_white {nullptr};

    wxDECLARE_EVENT_TABLE();
};
