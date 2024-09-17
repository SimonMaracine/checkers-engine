#include "main_window.hpp"

#include <iostream>
#include <cstring>
#include <algorithm>
#include <cassert>
#include <fstream>

#include <wx/statline.h>
#include <common/fen_string.hpp>

enum {
    START_ENGINE_BLACK = 10,
    START_ENGINE_WHITE,
    SHOW_INDICES,
    PLAY_POSITION,
    PLAY_POSITIONS,
    STOP
};

static const wxString ENGINE_BLACK {"Black engine: "};
static const wxString ENGINE_WHITE {"White engine: "};
static const wxString EVAL {"Eval: "};

wxBEGIN_EVENT_TABLE(MainWindow, wxFrame)
    EVT_MENU(START_ENGINE_BLACK, MainWindow::on_start_engine_black)
    EVT_MENU(START_ENGINE_WHITE, MainWindow::on_start_engine_white)
    EVT_MENU(SHOW_INDICES, MainWindow::on_show_indices)
    EVT_MENU(wxID_EXIT, MainWindow::on_exit)
    EVT_MENU(PLAY_POSITION, MainWindow::on_play_position)
    EVT_MENU(PLAY_POSITIONS, MainWindow::on_play_positions)
    EVT_MENU(STOP, MainWindow::on_stop)
    EVT_MENU(wxID_ABOUT, MainWindow::on_about)
    EVT_SIZE(MainWindow::on_window_resize)
    EVT_CLOSE(MainWindow::on_close)
wxEND_EVENT_TABLE()

MainWindow::MainWindow()
    : wxFrame(nullptr, wxID_ANY, "Checkers Tester") {
    SetMinSize(wxSize(896, 504));  // Set minimum size here to trigger a resize event after widgets creation
    setup_menubar();
    setup_widgets();
    Center();

    m_engine_black = std::make_unique<engine::Engine>([this](const auto& message, bool error) { on_engine_message(message, error, Player::Black); });
    m_engine_white = std::make_unique<engine::Engine>([this](const auto& message, bool error) { on_engine_message(message, error, Player::White); });
}

void MainWindow::setup_menubar() {
    wxMenu* men_tester {new wxMenu};
    men_tester->Append(START_ENGINE_BLACK, "Start Black Engine");
    men_tester->Append(START_ENGINE_WHITE, "Start White Engine");
    men_tester->Append(SHOW_INDICES, "Show Indices", wxEmptyString, wxITEM_CHECK);
    men_tester->Append(wxID_EXIT, "Exit");

    wxMenu* men_run {new wxMenu};
    m_btn_play_position = men_run->Append(PLAY_POSITION, "Play Position");
    m_btn_play_positions = men_run->Append(PLAY_POSITIONS, "Play Positions");
    m_btn_stop = men_run->Append(STOP, "Stop");

    m_btn_play_position->Enable(false);
    m_btn_play_positions->Enable(false);
    m_btn_stop->Enable(false);

    wxMenu* men_help {new wxMenu};
    men_help->Append(wxID_ABOUT, "About");

    wxMenuBar* menu_bar {new wxMenuBar};
    menu_bar->Append(men_tester, "Tester");
    menu_bar->Append(men_run, "Run");
    menu_bar->Append(men_help, "Help");

    SetMenuBar(menu_bar);
}

void MainWindow::setup_widgets() {
    wxBoxSizer* szr_main {new wxBoxSizer(wxHORIZONTAL)};

    {
        m_board = new board::CheckersBoard(this, -1, -1, 400,
            [this](const board::CheckersBoard::Move& move) {
                return on_piece_move(move);
            }
        );

        szr_main->Add(m_board, 3, wxEXPAND | wxALL);

        m_board->set_user_input(false);
    }

    szr_main->AddSpacer(30);

    {
        wxPanel* pnl_right_side {new wxPanel(this)};
        wxBoxSizer* szr_right_side {new wxBoxSizer(wxVERTICAL)};

        {
            wxBoxSizer* szr_game_state {new wxBoxSizer(wxVERTICAL)};
            m_pnl_game_state = new game_state::GameStatePanel(pnl_right_side, szr_game_state);
            m_pnl_game_state->SetSizer(szr_game_state);
            szr_right_side->Add(m_pnl_game_state);
        }

        szr_right_side->AddSpacer(10);
        szr_right_side->Add(new wxStaticLine(pnl_right_side), 0, wxEXPAND | wxRIGHT);
        szr_right_side->AddSpacer(10);

        {
            m_txt_engine_black = new wxStaticText(pnl_right_side, wxID_ANY, ENGINE_BLACK);
            szr_right_side->Add(m_txt_engine_black);

            m_txt_eval_black = new wxStaticText(pnl_right_side, wxID_ANY, EVAL + "0");
            szr_right_side->Add(m_txt_eval_black);
        }

        szr_right_side->AddSpacer(10);
        szr_right_side->Add(new wxStaticLine(pnl_right_side), 0, wxEXPAND | wxRIGHT);
        szr_right_side->AddSpacer(10);

        {
            m_pnl_parameters_black = new parameters::ParametersPanel(pnl_right_side, 30);
            m_pnl_parameters_black->set_sizer(new wxBoxSizer(wxVERTICAL));
            szr_right_side->Add(m_pnl_parameters_black, 1, wxEXPAND | wxALL);
        }

        szr_right_side->AddSpacer(10);
        szr_right_side->Add(new wxStaticLine(pnl_right_side), 0, wxEXPAND | wxRIGHT);
        szr_right_side->AddSpacer(10);

        {
            m_txt_engine_white = new wxStaticText(pnl_right_side, wxID_ANY, ENGINE_WHITE);
            szr_right_side->Add(m_txt_engine_white);

            m_txt_eval_white = new wxStaticText(pnl_right_side, wxID_ANY, EVAL + "0");
            szr_right_side->Add(m_txt_eval_white);
        }

        szr_right_side->AddSpacer(10);
        szr_right_side->Add(new wxStaticLine(pnl_right_side), 0, wxEXPAND | wxRIGHT);
        szr_right_side->AddSpacer(10);

        {
            m_pnl_parameters_white = new parameters::ParametersPanel(pnl_right_side, 30);
            m_pnl_parameters_white->set_sizer(new wxBoxSizer(wxVERTICAL));
            szr_right_side->Add(m_pnl_parameters_white, 1, wxEXPAND | wxALL);
        }

        pnl_right_side->SetSizer(szr_right_side);

        szr_main->Add(pnl_right_side, 2, wxEXPAND | wxRIGHT);
    }

    szr_main->AddSpacer(30);

    {
        m_pnl_moves_log = new moves_log::MovesLog(this);
        m_pnl_moves_log->set_sizer(new wxBoxSizer(wxVERTICAL));
        szr_main->Add(m_pnl_moves_log, 1, wxEXPAND | wxDOWN);
    }

    SetSizer(szr_main);
}

void MainWindow::on_exit(wxCommandEvent&) {
    stop_engine(m_engine_black);
    stop_engine(m_engine_white);

    wxExit();
}

void MainWindow::on_close(wxCloseEvent&) {
    stop_engine(m_engine_black);
    stop_engine(m_engine_white);

    wxExit();
}

void MainWindow::on_start_engine_black(wxCommandEvent&) {
    start_engine(m_engine_black);
}

void MainWindow::on_start_engine_white(wxCommandEvent&) {
    start_engine(m_engine_white);
}

void MainWindow::on_show_indices(wxCommandEvent&) {
    static bool show_indices {false};

    m_board->set_show_indices(!std::exchange(show_indices, !show_indices));
}

void MainWindow::on_about(wxCommandEvent&) {
    wxMessageBox(
        "Checkers Tester, comparing the strengths of two engines.",
        "About",
        wxOK | wxICON_INFORMATION
    );
}

void MainWindow::on_window_resize(wxSizeEvent& event) {
    // This function may be called before board is initialized
    if (m_board != nullptr) {
        Layout();
        m_board->set_board_size(get_ideal_board_size());
    }

    event.Skip();
}

void MainWindow::on_play_position(wxCommandEvent&) {
    FenString dialog {this, wxID_ANY};

    if (dialog.ShowModal() != wxID_OK) {
        return;
    }

    set_position(dialog.get_fen_string().ToStdString());

    try {
        m_engine_black->go(false);
    } catch (const engine::Engine::Error& e) {
        std::cerr << e.what() << '\n';
    }

    m_btn_play_position->Enable(false);
    m_btn_play_positions->Enable(false);
    m_btn_stop->Enable();
}

void MainWindow::on_play_positions(wxCommandEvent&) {
    wxFileDialog dialog {this};

    if (dialog.ShowModal() != wxID_OK) {
        return;
    }

    read_positions_file(dialog.GetPath().ToStdString());

    // TODO
}

void MainWindow::on_stop(wxCommandEvent&) {
    // TODO

    // try {
    //     m_engine_black->stop();
    //     m_engine_white->stop();
    // } catch (const engine::Engine::Error& e) {
    //     std::cerr << "Error stop: " << e.what() << '\n';
    // }
}

void MainWindow::on_piece_move(const board::CheckersBoard::Move& move) {
    m_pnl_moves_log->log_move(move);

    m_pnl_game_state->update(m_board);

    switch (board::CheckersBoard::opponent(m_board->get_player())) {
        case board::CheckersBoard::Player::Black:
            try {
                m_engine_white->move(board::CheckersBoard::move_to_string(move));
            } catch (const engine::Engine::Error& e) {
                std::cerr << e.what() << '\n';
            }

            break;
        case board::CheckersBoard::Player::White:
            try {
                m_engine_black->move(board::CheckersBoard::move_to_string(move));
            } catch (const engine::Engine::Error& e) {
                std::cerr << e.what() << '\n';
            }

            break;
    }

    if (m_board->get_game_over() != board::CheckersBoard::GameOver::None) {
        return;
    }

    switch (board::CheckersBoard::opponent(m_board->get_player())) {
        case board::CheckersBoard::Player::Black:
            try {
                m_engine_white->go(false);
            } catch (const engine::Engine::Error& e) {
                std::cerr << e.what() << '\n';
            }

            break;
        case board::CheckersBoard::Player::White:
            try {
                m_engine_black->go(false);
            } catch (const engine::Engine::Error& e) {
                std::cerr << e.what() << '\n';
            }

            break;
    }
}

void MainWindow::on_engine_message(const std::string& message, bool error, Player player) {
    if (error) {
        std::cerr << message << '\n';
        return;
    }

    parameters::ParametersPanel* pnl_parameters {player == Player::Black ? m_pnl_parameters_black : m_pnl_parameters_white};
    wxStaticText* txt_eval {player == Player::Black ? m_txt_eval_black : m_txt_eval_white};

    auto tokens {parse_message(message)};

    if (tokens.empty()) {
        return;
    }

    // Remove new line from the last token
    tokens.back() = tokens.back().substr(0, tokens.back().size() - 1);

    if (tokens.at(0) == "READY") {
        if (player == Player::Black) {
            initialize_engine(m_engine_black, m_txt_engine_black, ENGINE_BLACK, m_pnl_parameters_black);
        } else {
            initialize_engine(m_engine_white, m_txt_engine_white, ENGINE_WHITE, m_pnl_parameters_white);
        }
    } else if (tokens.at(0) == "BESTMOVE") {
        if (tokens.size() > 1) {
            m_board->play_move(tokens.at(1));
        }
    } else if (tokens.at(0) == "PARAMETERS") {
        std::vector<std::pair<std::string, std::string>> parameters;
        auto i {tokens.size() - 1};

        while (i > 0) {
            const auto name = tokens.at(tokens.size() - i--);
            const auto type = tokens.at(tokens.size() - i--);

            parameters.push_back(std::make_pair(name, type));
        }

        pnl_parameters->get_engine_parameters(std::move(parameters));
    } else if (tokens.at(0) == "PARAMETER") {
        pnl_parameters->add_parameter(tokens.at(1), tokens.at(2));

        Layout();
    } else if (tokens.at(0) == "INFO") {
        std::cout << message;  // It already has a new line

        txt_eval->SetLabelText(EVAL + tokens.at(4));
    }
}

void MainWindow::start_engine(const std::unique_ptr<engine::Engine>& engine) {
    wxFileDialog dialog {this};

    if (dialog.ShowModal() != wxID_OK) {
        return;
    }

    stop_engine(engine);

    try {
        engine->start_engine(dialog.GetPath().ToStdString(), dialog.GetFilename().ToStdString());
    } catch (const engine::Engine::Error& e) {
        std::cerr << e.what() << '\n';
    }
}

void MainWindow::stop_engine(const std::unique_ptr<engine::Engine>& engine) {
    try {
        engine->quit();
    } catch (const engine::Engine::Error& e) {
        std::cerr << e.what() << '\n';
    }

    try {
        engine->stop_engine();
    } catch (const engine::Engine::Error& e) {
        std::cerr << e.what() << '\n';
    }
}

void MainWindow::initialize_engine(
    const std::unique_ptr<engine::Engine>& engine,
    wxStaticText* txt_engine,
    const wxString& text,
    parameters::ParametersPanel* pnl_parameters
) {
    try {
        engine->init();
    } catch (const engine::Engine::Error& e) {
        std::cerr << e.what() << '\n';
        return;
    }

    txt_engine->SetLabelText(text + engine->get_name());

    pnl_parameters->clear_parameters();
    pnl_parameters->set_engine(engine.get());

    try {
        engine->getparameters();
    } catch (const engine::Engine::Error& e) {
        std::cerr << e.what() << '\n';
    }

    if (m_engine_black->is_started() && m_engine_white->is_started()) {
        m_btn_play_position->Enable();
        m_btn_play_positions->Enable();
    }
}

void MainWindow::set_position(const std::optional<std::string>& fen_string) {
    if (!fen_string) {
        m_board->reset_position();
    } else {
        m_board->set_position(*fen_string);
    }

    m_pnl_moves_log->clear_log();

    m_pnl_game_state->reset(m_board);

    try {
        m_engine_black->newgame(fen_string);
        m_engine_white->newgame(fen_string);
    } catch (const engine::Engine::Error& e) {
        std::cerr << e.what() << '\n';
    }
}

int MainWindow::get_ideal_board_size() {
    const wxSize size {m_board->GetSize()};

    return std::min(size.GetHeight(), size.GetWidth());
}

std::vector<std::string> MainWindow::parse_message(const std::string& message) {
    std::vector<std::string> tokens;

    std::string mutable_buffer {message};

    char* token {std::strtok(mutable_buffer.data(), " \t")};

    while (token != nullptr) {
        tokens.emplace_back(token);

        token = std::strtok(nullptr, " \t");
    }

    return tokens;
}

void MainWindow::read_positions_file(const std::string& file_path) {
    std::ifstream stream {file_path};

    if (!stream.is_open()) {

    }

    // TODO
}
