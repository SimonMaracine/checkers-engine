#include "main_window.hpp"

#include <iostream>
#include <cstring>
#include <algorithm>
#include <cassert>

#include <wx/statline.h>

#include <common/fen_string.hpp>

enum {
    START_ENGINE_BLACK = 10,
    START_ENGINE_WHITE,
    SHOW_INDICES,
    PLAY_POSITION,
    PLAY_100_POSITIONS,
    STOP
};

static const wxString ENGINE_BLACK {"Black engine: "};
static const wxString ENGINE_WHITE {"White engine: "};

wxBEGIN_EVENT_TABLE(MainWindow, wxFrame)
    EVT_MENU(START_ENGINE_BLACK, MainWindow::on_start_engine_black)
    EVT_MENU(START_ENGINE_WHITE, MainWindow::on_start_engine_white)
    EVT_MENU(SHOW_INDICES, MainWindow::on_show_indices)
    EVT_MENU(wxID_EXIT, MainWindow::on_exit)
    EVT_MENU(PLAY_POSITION, MainWindow::on_play_position)
    EVT_MENU(PLAY_100_POSITIONS, MainWindow::on_play_100_positions)
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

    engine_black = std::make_unique<engine::Engine>([this](const auto& message) { on_engine_message(message, Player::Black); });
    engine_white = std::make_unique<engine::Engine>([this](const auto& message) { on_engine_message(message, Player::White); });
}

void MainWindow::setup_menubar() {
    wxMenu* men_tester {new wxMenu};
    men_tester->Append(START_ENGINE_BLACK, "Start Black Engine");
    men_tester->Append(START_ENGINE_WHITE, "Start White Engine");
    men_tester->Append(SHOW_INDICES, "Show Indices", wxEmptyString, wxITEM_CHECK);
    men_tester->Append(wxID_EXIT, "Exit");

    wxMenu* men_run {new wxMenu};
    btn_play_position = men_run->Append(PLAY_POSITION, "Play Position");
    btn_play_100_positions = men_run->Append(PLAY_100_POSITIONS, "Play 100 Positions");
    btn_stop = men_run->Append(STOP, "Stop");

    btn_play_position->Enable(false);
    btn_play_100_positions->Enable(false);
    btn_stop->Enable(false);

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
        board = new board::CheckersBoard(this, -1, -1, 400,
            [this](const board::CheckersBoard::Move& move) {
                return on_piece_move(move);
            }
        );

        szr_main->Add(board, 3, wxEXPAND | wxALL);

        board->set_user_input(false);
    }

    szr_main->AddSpacer(30);

    {
        wxPanel* pnl_right_side {new wxPanel(this)};
        wxBoxSizer* szr_right_side {new wxBoxSizer(wxVERTICAL)};

        {
            wxBoxSizer* szr_game_state {new wxBoxSizer(wxVERTICAL)};
            pnl_game_state = new game_state::GameStatePanel(pnl_right_side, szr_game_state);
            pnl_game_state->SetSizer(szr_game_state);
            szr_right_side->Add(pnl_game_state);
        }

        szr_right_side->AddSpacer(10);
        szr_right_side->Add(new wxStaticLine(pnl_right_side), 0, wxEXPAND | wxRIGHT);
        szr_right_side->AddSpacer(10);

        {
            txt_engine_black = new wxStaticText(pnl_right_side, wxID_ANY, ENGINE_BLACK);
            szr_right_side->Add(txt_engine_black);
        }

        szr_right_side->AddSpacer(10);
        szr_right_side->Add(new wxStaticLine(pnl_right_side), 0, wxEXPAND | wxRIGHT);
        szr_right_side->AddSpacer(10);

        {
            pnl_parameters_black = new parameters::ParametersPanel(pnl_right_side, 30);
            pnl_parameters_black->set_sizer(new wxBoxSizer(wxVERTICAL));
            szr_right_side->Add(pnl_parameters_black, 1, wxEXPAND | wxALL);
        }

        szr_right_side->AddSpacer(10);
        szr_right_side->Add(new wxStaticLine(pnl_right_side), 0, wxEXPAND | wxRIGHT);
        szr_right_side->AddSpacer(10);

        {
            txt_engine_white = new wxStaticText(pnl_right_side, wxID_ANY, ENGINE_WHITE);
            szr_right_side->Add(txt_engine_white);
        }

        szr_right_side->AddSpacer(10);
        szr_right_side->Add(new wxStaticLine(pnl_right_side), 0, wxEXPAND | wxRIGHT);
        szr_right_side->AddSpacer(10);

        {
            pnl_parameters_white = new parameters::ParametersPanel(pnl_right_side, 30);
            pnl_parameters_white->set_sizer(new wxBoxSizer(wxVERTICAL));
            szr_right_side->Add(pnl_parameters_white, 1, wxEXPAND | wxALL);
        }

        pnl_right_side->SetSizer(szr_right_side);

        szr_main->Add(pnl_right_side, 2, wxEXPAND | wxRIGHT);
    }

    szr_main->AddSpacer(30);

    {
        pnl_moves_log = new moves_log::MovesLog(this);
        pnl_moves_log->set_sizer(new wxBoxSizer(wxVERTICAL));
        szr_main->Add(pnl_moves_log, 1, wxEXPAND | wxDOWN);
    }

    SetSizer(szr_main);
}

void MainWindow::on_exit(wxCommandEvent&) {
    try {
        engine_black->quit();
        engine_white->quit();
    } catch (const engine::Engine::Error& e) {
        std::cerr << "Error quit: " << e.what() << '\n';
    }

    wxExit();
}

void MainWindow::on_close(wxCloseEvent&) {
    try {
        engine_black->quit();
        engine_white->quit();
    } catch (const engine::Engine::Error& e) {
        std::cerr << "Error quit: " << e.what() << '\n';
    }

    wxExit();
}

void MainWindow::on_start_engine_black(wxCommandEvent&) {
    start_engine(engine_black, txt_engine_black, ENGINE_BLACK, pnl_parameters_black);
}

void MainWindow::on_start_engine_white(wxCommandEvent&) {
    start_engine(engine_white, txt_engine_white, ENGINE_WHITE, pnl_parameters_white);
}

void MainWindow::on_show_indices(wxCommandEvent&) {
    static bool show_indices {false};

    board->set_show_indices(!std::exchange(show_indices, !show_indices));
}

void MainWindow::on_about(wxCommandEvent&) {
    wxMessageBox(
        "Checkers Tester, comparing the strength of two engines.",
        "About",
        wxOK | wxICON_INFORMATION
    );
}

void MainWindow::on_window_resize(wxSizeEvent& event) {
    // This function may be called before board is initialized
    if (board != nullptr) {
        Layout();
        board->set_board_size(get_ideal_board_size());
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
        engine_black->go(false);
    } catch (const engine::Engine::Error& e) {
        std::cerr << "Error go: " << e.what() << '\n';
    }

    btn_play_position->Enable(false);
    btn_play_100_positions->Enable(false);
    btn_stop->Enable();
}

void MainWindow::on_play_100_positions(wxCommandEvent&) {
    // TODO
}

void MainWindow::on_stop(wxCommandEvent&) {
    // TODO

    // try {
    //     engine_black->stop();
    //     engine_white->stop();
    // } catch (const engine::Engine::Error& e) {
    //     std::cerr << "Error stop: " << e.what() << '\n';
    // }

    // btn_stop->Enable(false);
    // btn_play_position->Enable();
    // btn_play_100_positions->Enable();
}

void MainWindow::start_engine(
    std::unique_ptr<engine::Engine>& engine,
    wxStaticText* txt_engine,
    const wxString& text,
    parameters::ParametersPanel* pnl_parameters
) {
    wxFileDialog dialog {this};

    if (dialog.ShowModal() != wxID_OK) {
        return;
    }

    try {
        engine->quit();
    } catch (const engine::Engine::Error& e) {
        std::cerr << "Error quit: " << e.what() << '\n';
        return;
    }

    try {
        engine->init(dialog.GetPath().ToStdString());
    } catch (const engine::Engine::Error& e) {
        std::cerr << "Error init: " << e.what() << '\n';
        return;
    }

    txt_engine->SetLabelText(text + dialog.GetFilename());

    pnl_parameters->clear_parameters();
    pnl_parameters->set_engine(engine.get());

    try {
        engine->getparameters();
    } catch (const engine::Engine::Error& e) {
        std::cerr << "Error getparameters: " << e.what() << '\n';
    }

    if (engine_black->is_started() && engine_white->is_started()) {
        btn_play_position->Enable();
        btn_play_100_positions->Enable();
    }
}

void MainWindow::on_piece_move(const board::CheckersBoard::Move& move) {
    pnl_moves_log->log_move(move);

    pnl_game_state->update(board);

    switch (board::CheckersBoard::opponent(board->get_player())) {
        case board::CheckersBoard::Player::Black:
            try {
                engine_white->move(board::CheckersBoard::move_to_string(move));
            } catch (const engine::Engine::Error& e) {
                std::cerr << "Error move: " << e.what() << '\n';
            }

            break;
        case board::CheckersBoard::Player::White:
            try {
                engine_black->move(board::CheckersBoard::move_to_string(move));
            } catch (const engine::Engine::Error& e) {
                std::cerr << "Error move: " << e.what() << '\n';
            }

            break;
    }

    if (board->get_game_over() != board::CheckersBoard::GameOver::None) {
        return;
    }

    switch (board::CheckersBoard::opponent(board->get_player())) {
        case board::CheckersBoard::Player::Black:
            try {
                engine_white->go(false);
            } catch (const engine::Engine::Error& e) {
                std::cerr << "Error move: " << e.what() << '\n';
            }

            break;
        case board::CheckersBoard::Player::White:
            try {
                engine_black->go(false);
            } catch (const engine::Engine::Error& e) {
                std::cerr << "Error move: " << e.what() << '\n';
            }

            break;
    }
}

void MainWindow::on_engine_message(const std::string& message, Player player) {
    process_engine_message(message, player);
}

void MainWindow::set_position(const std::optional<std::string>& fen_string) {
    if (!fen_string) {
        board->reset_position();
    } else {
        board->set_position(*fen_string);
    }

    pnl_moves_log->clear_log();

    pnl_game_state->reset(board);

    try {
        engine_black->newgame(fen_string);
        engine_white->newgame(fen_string);
    } catch (const engine::Engine::Error& e) {
        std::cerr << "Error newgame: " << e.what() << '\n';
    }
}

int MainWindow::get_ideal_board_size() {
    const wxSize size {board->GetSize()};

    return std::min(size.GetHeight(), size.GetWidth());
}

void MainWindow::process_engine_message(const std::string& message, Player player) {
    parameters::ParametersPanel* pnl_parameters {player == Player::Black ? pnl_parameters_black : pnl_parameters_white};

    auto tokens {parse_message(message)};

    if (tokens.empty()) {
        return;
    }

    // Remove new line from the last token
    tokens.back() = tokens.back().substr(0u, tokens.back().size() - 1u);

    if (tokens.at(0u) == "BESTMOVE") {
        if (tokens.size() > 1u) {
            board->play_move(tokens.at(1u));
        }
    } else if (tokens.at(0u) == "PARAMETERS") {
        std::vector<std::pair<std::string, std::string>> parameters;
        auto i {tokens.size() - 1u};

        while (i > 0u) {
            const auto name = tokens.at(tokens.size() - i--);
            const auto type = tokens.at(tokens.size() - i--);

            parameters.push_back(std::make_pair(name, type));
        }

        pnl_parameters->get_engine_parameters(std::move(parameters));
    } else if (tokens.at(0u) == "PARAMETER") {
        pnl_parameters->add_parameter(tokens.at(1u), tokens.at(2u));

        Layout();
    }
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
