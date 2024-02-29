#include "main_window.hpp"

#include <iostream>
#include <cstring>
#include <algorithm>
#include <cassert>

#include <wx/statline.h>

#include <common/fen_string.hpp>

enum {
    START_ENGINE = 10,
    RESET_POSITION,
    SET_POSITION,
    SHOW_INDICES,
    BLACK,
    WHITE,
    STOP,
    CONTINUE
};

static const wxString ENGINE {"Engine: "};

wxBEGIN_EVENT_TABLE(MainWindow, wxFrame)
    EVT_MENU(START_ENGINE, MainWindow::on_start_engine)
    EVT_MENU(RESET_POSITION, MainWindow::on_reset_position)
    EVT_MENU(SET_POSITION, MainWindow::on_set_position)
    EVT_MENU(SHOW_INDICES, MainWindow::on_show_indices)
    EVT_MENU(wxID_EXIT, MainWindow::on_exit)
    EVT_MENU(wxID_ABOUT, MainWindow::on_about)
    EVT_SIZE(MainWindow::on_window_resize)
    EVT_RADIOBUTTON(BLACK, MainWindow::on_black_change)
    EVT_RADIOBUTTON(WHITE, MainWindow::on_white_change)
    EVT_BUTTON(STOP, MainWindow::on_stop)
    EVT_BUTTON(CONTINUE, MainWindow::on_continue)
    EVT_CLOSE(MainWindow::on_close)
wxEND_EVENT_TABLE()

MainWindow::MainWindow()
    : wxFrame(nullptr, wxID_ANY, "Checkers Player") {
    SetMinSize(wxSize(896, 504));  // Set minimum size here to trigger a resize event after widgets creation
    setup_menubar();
    setup_widgets();
    Center();

    engine = std::make_unique<engine::Engine>([this](const auto& message) { on_engine_message(message); });
}

void MainWindow::setup_menubar() {
    wxMenu* men_file {new wxMenu};
    men_file->Append(START_ENGINE, "Start Engine");
    men_file->Append(RESET_POSITION, "Reset Position");
    men_file->Append(SET_POSITION, "Set Position");
    men_file->Append(SHOW_INDICES, "Show Indices");
    men_file->Append(wxID_EXIT, "Exit");

    wxMenu* men_help {new wxMenu};
    men_help->Append(wxID_ABOUT, "About");

    wxMenuBar* menu_bar {new wxMenuBar};
    menu_bar->Append(men_file, "File");
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

        // Initially user input is disabled until an engine is loaded
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
            wxPanel* pnl_players {new wxPanel(pnl_right_side)};
            wxBoxSizer* szr_players {new wxBoxSizer(wxHORIZONTAL)};

            {
                wxPanel* pnl_black {new wxPanel(pnl_players)};
                wxBoxSizer* szr_black {new wxBoxSizer(wxVERTICAL)};

                szr_black->Add(new wxStaticText(pnl_black, wxID_ANY, "Black"), 1);

                btn_black_human = new wxRadioButton(pnl_black, BLACK, "Human");
                szr_black->Add(btn_black_human, 1);

                btn_black_computer = new wxRadioButton(pnl_black, BLACK, "Computer");
                szr_black->Add(btn_black_computer, 1);

                pnl_black->SetSizer(szr_black);

                szr_players->Add(pnl_black, 1);
            }

            {
                wxPanel* pnl_white {new wxPanel(pnl_players)};
                wxBoxSizer* szr_white {new wxBoxSizer(wxVERTICAL)};

                szr_white->Add(new wxStaticText(pnl_white, wxID_ANY, "White"), 1);

                btn_white_human = new wxRadioButton(pnl_white, WHITE, "Human");
                szr_white->Add(btn_white_human, 1);

                btn_white_computer = new wxRadioButton(pnl_white, WHITE, "Computer");
                btn_white_computer->SetValue(true);
                szr_white->Add(btn_white_computer, 1);

                pnl_white->SetSizer(szr_white);

                szr_players->Add(pnl_white, 1);
            }

            pnl_players->SetSizer(szr_players);

            szr_right_side->Add(pnl_players);

            // Initially these are disabled until an engine is loaded
            btn_black_human->Disable();
            btn_black_computer->Disable();
            btn_white_human->Disable();
            btn_white_computer->Disable();
        }


        szr_right_side->AddSpacer(10);
        szr_right_side->Add(new wxStaticLine(pnl_right_side), 0, wxEXPAND | wxRIGHT);
        szr_right_side->AddSpacer(10);

        {
            wxPanel* pnl_control_buttons {new wxPanel(pnl_right_side)};
            wxBoxSizer* szr_control_buttons {new wxBoxSizer(wxHORIZONTAL)};

            btn_stop = new wxButton(pnl_control_buttons, STOP, "Stop");
            szr_control_buttons->Add(btn_stop, 1);

            szr_control_buttons->AddSpacer(10);

            btn_continue = new wxButton(pnl_control_buttons, CONTINUE, "Continue");
            szr_control_buttons->Add(btn_continue, 1);

            pnl_control_buttons->SetSizer(szr_control_buttons);

            szr_right_side->Add(pnl_control_buttons);

            // Initially these are disabled until an engine is loaded
            btn_stop->Disable();
            btn_continue->Disable();
        }

        szr_right_side->AddSpacer(10);
        szr_right_side->Add(new wxStaticLine(pnl_right_side), 0, wxEXPAND | wxRIGHT);
        szr_right_side->AddSpacer(10);

        {
            txt_engine = new wxStaticText(pnl_right_side, wxID_ANY, ENGINE);
            szr_right_side->Add(txt_engine);
        }

        szr_right_side->AddSpacer(10);
        szr_right_side->Add(new wxStaticLine(pnl_right_side), 0, wxEXPAND | wxRIGHT);
        szr_right_side->AddSpacer(10);

        {
            pnl_parameters = new parameters::ParametersPanel(pnl_right_side, 30);

            pnl_parameters->set_sizer(new wxBoxSizer(wxVERTICAL));

            szr_right_side->Add(pnl_parameters, 1, wxEXPAND | wxALL);
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
        engine->quit();
    } catch (const engine::Engine::Error& e) {
        std::cerr << "Error quit: " << e.what() << '\n';
    }

    wxExit();
}

void MainWindow::on_close(wxCloseEvent&) {
    try {
        engine->quit();
    } catch (const engine::Engine::Error& e) {
        std::cerr << "Error quit: " << e.what() << '\n';
    }

    wxExit();
}

void MainWindow::on_start_engine(wxCommandEvent&) {
    wxFileDialog dialog {this};

    if (dialog.ShowModal() == wxID_OK) {
        // Reset the board here in case the engine is reloaded
        set_position(std::nullopt);

        // Also stop the current engine
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

        txt_engine->SetLabelText(ENGINE + dialog.GetFilename());

        pnl_parameters->clear_parameters();
        pnl_parameters->set_engine(engine.get());

        try {
            engine->getparameters();
        } catch (const engine::Engine::Error& e) {
            std::cerr << "Error getparameters: " << e.what() << '\n';
        }

        // Don't tell the engine to go now, if it's their turn; wait until the continue button is pressed

        if (get_player_type(board->get_player()) == PlayerType::Human) {
            // Finally enable user input
            board->set_user_input(true);
        }

        // Finally enable these as well
        btn_black_human->Enable();
        btn_black_computer->Enable();
        btn_white_human->Enable();
        btn_white_computer->Enable();

        btn_stop->Enable();
        btn_continue->Enable();
    }
}

void MainWindow::on_reset_position(wxCommandEvent&) {
    set_position(std::nullopt);
}

void MainWindow::on_set_position(wxCommandEvent&) {
    FenString dialog {this, wxID_ANY};

    if (dialog.ShowModal() == wxID_OK) {
        set_position(dialog.get_fen_string().ToStdString());
    }
}

void MainWindow::on_show_indices(wxCommandEvent&) {
    static bool show_indices {false};

    board->set_show_indices(!std::exchange(show_indices, !show_indices));
}

void MainWindow::on_about(wxCommandEvent&) {
    wxMessageBox(
        "Checkers Player, an implementation of the game of checkers.",
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

void MainWindow::on_black_change(wxCommandEvent&) {
    if (btn_black_human->GetValue()) {
        black = PlayerType::Human;

        if (board->get_player() == board::CheckersBoard::Player::Black) {
            board->set_user_input(true);
        }
    } else {
        black = PlayerType::Computer;

        if (board->get_player() == board::CheckersBoard::Player::Black) {
            board->set_user_input(false);
        }
    }
}

void MainWindow::on_white_change(wxCommandEvent&) {
    if (btn_white_human->GetValue()) {
        white = PlayerType::Human;

        if (board->get_player() == board::CheckersBoard::Player::White) {
            board->set_user_input(true);
        }
    } else {
        white = PlayerType::Computer;

        if (board->get_player() == board::CheckersBoard::Player::White) {
            board->set_user_input(false);
        }
    }
}

void MainWindow::on_stop(wxCommandEvent&) {
    try {
        engine->stop();
    } catch (const engine::Engine::Error& e) {
        std::cerr << "Error stop: " << e.what() << '\n';
    }
}

void MainWindow::on_continue(wxCommandEvent&) {
    if (get_player_type(board->get_player()) != PlayerType::Computer) {
        return;
    }

    // FIXME don't call go multiple times

    try {
        engine->go(false);
    } catch (const engine::Engine::Error& e) {
        std::cerr << "Error go: " << e.what() << '\n';
    }
}

void MainWindow::on_piece_move(const board::CheckersBoard::Move& move) {
    pnl_moves_log->log_move(move);

    pnl_game_state->update(board);

    if (get_player_type(board->get_player()) == PlayerType::Computer) {
        if (get_player_type(board::CheckersBoard::opponent(board->get_player())) == PlayerType::Human) {
            try {
                engine->move(board::CheckersBoard::move_to_string(move));
            } catch (const engine::Engine::Error& e) {
                std::cerr << "Error move: " << e.what() << '\n';
            }
        }
    }

    if (board->get_game_over() != board::CheckersBoard::GameOver::None) {
        return;
    }

    if (get_player_type(board->get_player()) == PlayerType::Computer) {
        try {
            engine->go(false);
        } catch (const engine::Engine::Error& e) {
            std::cerr << "Error go: " << e.what() << '\n';
        }

        board->set_user_input(false);
    } else {
        board->set_user_input(true);
    }

    // It's okay to disable these over and over again
    btn_black_human->Disable();
    btn_black_computer->Disable();
    btn_white_human->Disable();
    btn_white_computer->Disable();
}

void MainWindow::on_engine_message(const std::string& message) {
    process_engine_message(message);
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
        engine->newgame(fen_string);
    } catch (const engine::Engine::Error& e) {
        std::cerr << "Error newgame: " << e.what() << '\n';
    }

    if (get_player_type(board->get_player()) == PlayerType::Computer) {
        board->set_user_input(false);
    } else {
        board->set_user_input(true);
    }

    btn_black_human->Enable();
    btn_black_computer->Enable();
    btn_white_human->Enable();
    btn_white_computer->Enable();
}

int MainWindow::get_ideal_board_size() {
    const wxSize size {board->GetSize()};

    return std::min(size.GetHeight(), size.GetWidth());
}

MainWindow::PlayerType MainWindow::get_player_type(board::CheckersBoard::Player player) {
    const PlayerType PLAYERS[2u] { black, white };

    return PLAYERS[static_cast<unsigned int>(player) - 1u];
}

void MainWindow::process_engine_message(const std::string& message) {
    auto tokens {parse_message(message)};

    if (tokens.empty()) {
        return;
    }

    // Remove new line from the last token
    tokens.back() = tokens.back().substr(0u, tokens.back().size() - 1u);

    if (tokens.at(0u) == "WARNING") {  // TODO maybe get rid of this message
        if (tokens.size() > 1u) {
            std::cerr << "Engine warning: " << tokens.at(1u) << '\n';
        }
    } else if (tokens.at(0u) == "BESTMOVE") {
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
