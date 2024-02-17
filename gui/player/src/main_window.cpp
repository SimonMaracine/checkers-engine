#include "main_window.hpp"

#include <iostream>
#include <cstdio>
#include <cstring>
#include <algorithm>
#include <optional>
#include <cassert>

#include <wx/statline.h>

#include "fen_string_dialog.hpp"

// FIXME need to make a stop and continue button for the engine and to also update the protocol; engine should no longer automatically play moves

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

static const wxString STATUS {"Status: "};
static const wxString PLAYER {"Player: "};
static const wxString PLIES_WITHOUT_ADVANCEMENT {"Plies without advancement: "};
static const wxString REPETITION_SIZE {"Repetition size: "};
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
    board = new board::CheckersBoard(this, -1, -1, 400,
        [this](const board::CheckersBoard::Move& move) {
            return on_piece_move(move);
        }
    );

    wxPanel* pnl_right_side {new wxPanel(this)};
    wxBoxSizer* szr_right_side {new wxBoxSizer(wxVERTICAL)};

    txt_status = new wxStaticText(pnl_right_side, wxID_ANY, STATUS + "game in progress");
    szr_right_side->Add(txt_status, 1);

    txt_player = new wxStaticText(pnl_right_side, wxID_ANY, PLAYER + "black");
    szr_right_side->Add(txt_player, 1);

    txt_plies_without_advancement = new wxStaticText(pnl_right_side, wxID_ANY, PLIES_WITHOUT_ADVANCEMENT + "0");
    szr_right_side->Add(txt_plies_without_advancement, 1);

    txt_repetition_size = new wxStaticText(pnl_right_side, wxID_ANY, REPETITION_SIZE + "0");
    szr_right_side->Add(txt_repetition_size, 1);

    szr_right_side->AddSpacer(10);
    szr_right_side->Add(new wxStaticLine(pnl_right_side), 0, wxEXPAND | wxRIGHT);
    szr_right_side->AddSpacer(10);

    wxPanel* pnl_players {new wxPanel(pnl_right_side)};
    wxBoxSizer* szr_players {new wxBoxSizer(wxHORIZONTAL)};

    wxPanel* pnl_black {new wxPanel(pnl_players)};
    wxBoxSizer* szr_black {new wxBoxSizer(wxVERTICAL)};

    szr_black->Add(new wxStaticText(pnl_black, wxID_ANY, "Black"), 1);

    btn_black_human = new wxRadioButton(pnl_black, BLACK, "Human");
    szr_black->Add(btn_black_human, 1);

    btn_black_computer = new wxRadioButton(pnl_black, BLACK, "Computer");
    szr_black->Add(btn_black_computer, 1);

    pnl_black->SetSizer(szr_black);

    szr_players->Add(pnl_black, 1);

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

    pnl_players->SetSizer(szr_players);

    szr_right_side->Add(pnl_players);

    szr_right_side->AddSpacer(10);
    szr_right_side->Add(new wxStaticLine(pnl_right_side), 0, wxEXPAND | wxRIGHT);
    szr_right_side->AddSpacer(10);

    wxPanel* pnl_control_buttons {new wxPanel(pnl_right_side)};
    wxBoxSizer* szr_control_buttons {new wxBoxSizer(wxHORIZONTAL)};

    btn_stop = new wxButton(pnl_control_buttons, STOP, "Stop");
    szr_control_buttons->Add(btn_stop, 1);

    szr_control_buttons->AddSpacer(10);

    btn_continue = new wxButton(pnl_control_buttons, CONTINUE, "Continue");
    szr_control_buttons->Add(btn_continue, 1);

    pnl_control_buttons->SetSizer(szr_control_buttons);

    szr_right_side->Add(pnl_control_buttons);

    szr_right_side->AddSpacer(10);
    szr_right_side->Add(new wxStaticLine(pnl_right_side), 0, wxEXPAND | wxRIGHT);
    szr_right_side->AddSpacer(10);

    txt_engine = new wxStaticText(pnl_right_side, wxID_ANY, ENGINE);
    szr_right_side->Add(txt_engine, 1);

    szr_right_side->AddSpacer(10);
    szr_right_side->Add(new wxStaticLine(pnl_right_side), 0, wxEXPAND | wxRIGHT);
    szr_right_side->AddSpacer(10);

    pnl_parameters = new parameters::ParametersPanel(pnl_right_side, 30);

    pnl_parameters->set_sizer(new wxBoxSizer(wxVERTICAL));

    szr_right_side->Add(pnl_parameters);

    pnl_right_side->SetSizer(szr_right_side);

    pnl_moves = new wxScrolledWindow(this);
    szr_moves = new wxBoxSizer(wxVERTICAL);

    pnl_moves->SetScrollRate(0, 10);
    pnl_moves->SetSizer(szr_moves);

    wxBoxSizer* szr_main {new wxBoxSizer(wxHORIZONTAL)};

    szr_main->Add(board, 3, wxEXPAND | wxALL);
    szr_main->AddSpacer(30);
    szr_main->Add(pnl_right_side, 2);
    szr_main->AddSpacer(30);
    szr_main->Add(pnl_moves, 1, wxEXPAND | wxDOWN);

    SetSizer(szr_main);
}

void MainWindow::on_exit(wxCommandEvent&) {
    engine->quit();

    wxExit();
}

void MainWindow::on_close(wxCloseEvent&) {
    engine->quit();

    wxExit();
}

void MainWindow::on_start_engine(wxCommandEvent&) {
    wxFileDialog dialog {this};

    if (dialog.ShowModal() == wxID_OK) {
        engine->quit();

        txt_engine->SetLabelText(ENGINE + dialog.GetFilename());

        try {
            engine->init(dialog.GetPath().ToStdString());
        } catch (int) {
            std::cout << "Error creating process\n";
            return;
        }

        // FIXME call newgame with the current position

        pnl_parameters->set_engine(engine.get());

        engine->getparameters();

        if (get_player_role(board->get_player()) == Player::Computer) {
            if (board->get_game_over() == board::CheckersBoard::GameOver::None) {
                engine->go(false);
            }
        }
    }
}

void MainWindow::on_reset_position(wxCommandEvent&) {
    board->reset_position();
    clear_moves_log();

    txt_status->SetLabelText(STATUS + "game in progress");
    txt_player->SetLabelText(PLAYER + "black");
    txt_plies_without_advancement->SetLabelText(PLIES_WITHOUT_ADVANCEMENT + "0");
    txt_repetition_size->SetLabelText(REPETITION_SIZE + "0");

    engine->newgame(std::nullopt);

    if (get_player_role(board->get_player()) == Player::Computer) {
        board->set_user_input(false);
        engine->go(false);
    } else {
        board->set_user_input(true);
    }

    btn_black_human->Enable();
    btn_black_computer->Enable();
    btn_white_human->Enable();
    btn_white_computer->Enable();
}

void MainWindow::on_set_position(wxCommandEvent&) {
    FenStringDialog dialog {this, wxID_ANY};

    if (dialog.ShowModal() == wxID_OK) {
        board->set_position(dialog.get_fen_string().ToStdString());
        clear_moves_log();

        txt_status->SetLabelText(STATUS + "game in progress");
        txt_player->SetLabelText(PLAYER + (board->get_player() == board::CheckersBoard::Player::Black ? "black" : "white"));
        txt_plies_without_advancement->SetLabelText(PLIES_WITHOUT_ADVANCEMENT + "0");
        txt_repetition_size->SetLabelText(REPETITION_SIZE + "0");

        engine->newgame(std::make_optional(dialog.get_fen_string().ToStdString()));

        if (get_player_role(board->get_player()) == Player::Computer) {
            board->set_user_input(false);
            engine->go(false);
        } else {
            board->set_user_input(true);
        }

        btn_black_human->Enable();
        btn_black_computer->Enable();
        btn_white_human->Enable();
        btn_white_computer->Enable();
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
        black = Player::Human;

        if (board->get_player() == board::CheckersBoard::Player::Black) {
            board->set_user_input(true);
        }
    } else {
        black = Player::Computer;

        if (board->get_player() == board::CheckersBoard::Player::Black) {
            board->set_user_input(false);

            if (board->get_game_over() == board::CheckersBoard::GameOver::None) {
                engine->go(false);
            }
        }
    }
}

void MainWindow::on_white_change(wxCommandEvent&) {
    if (btn_white_human->GetValue()) {
        white = Player::Human;

        if (board->get_player() == board::CheckersBoard::Player::White) {
            board->set_user_input(true);
        }
    } else {
        white = Player::Computer;

        if (board->get_player() == board::CheckersBoard::Player::White) {
            board->set_user_input(false);

            if (board->get_game_over() == board::CheckersBoard::GameOver::None) {
                engine->go(false);
            }
        }
    }
}

void MainWindow::on_stop(wxCommandEvent&) {
    engine->stop();
}

void MainWindow::on_continue(wxCommandEvent&) {

}

void MainWindow::on_piece_move(const board::CheckersBoard::Move& move) {
    log_move(move);

    txt_status->SetLabelText(STATUS + game_over_text());
    txt_player->SetLabelText(PLAYER + (board->get_player() == board::CheckersBoard::Player::Black ? "black" : "white"));
    txt_plies_without_advancement->SetLabelText(PLIES_WITHOUT_ADVANCEMENT + wxString::Format("%u", board->get_plies_without_advancement()));
    txt_repetition_size->SetLabelText(REPETITION_SIZE + wxString::Format("%zu", board->get_repetition_size()));

    if (get_player_role(board->get_player()) == Player::Computer) {
        if (get_player_role(board::CheckersBoard::opponent(board->get_player())) == Player::Human) {
            engine->move(board::CheckersBoard::move_to_string(move));
        }
    }

    if (board->get_game_over() != board::CheckersBoard::GameOver::None) {
        return;
    }

    if (get_player_role(board->get_player()) == Player::Computer) {
        engine->go(false);

        board->set_user_input(false);
    } else {
        board->set_user_input(true);
    }

    btn_black_human->Disable();
    btn_black_computer->Disable();
    btn_white_human->Disable();
    btn_white_computer->Disable();
}

void MainWindow::on_engine_message(const std::string& message) {
    process_engine_message(message);
}

int MainWindow::get_ideal_board_size() {
    const wxSize size {board->GetSize()};

    return std::min(size.GetHeight(), size.GetWidth());
}

const char* MainWindow::game_over_text() {
    switch (board->get_game_over()) {
        case board::CheckersBoard::GameOver::None:
            return "game in progress";
        case board::CheckersBoard::GameOver::WinnerBlack:
            return "game over (winner black)";
        case board::CheckersBoard::GameOver::WinnerWhite:
            return "game over (winner white)";
        case board::CheckersBoard::GameOver::Tie:
            return "game over (tie)";
    }

    return nullptr;
}

MainWindow::Player MainWindow::get_player_role(board::CheckersBoard::Player player) {
    const Player PLAYERS[2u] { black, white };

    return PLAYERS[static_cast<unsigned int>(player) - 1u];
}

void MainWindow::process_engine_message(const std::string& message) {
    auto tokens {parse_message(message)};

    if (tokens.empty()) {
        return;
    }

    // Remove new line from the last token
    tokens.back() = tokens.back().substr(0u, tokens.back().size() - 1u);

    if (tokens.at(0u) == "WARNING") {
        if (tokens.size() > 1u) {
            std::cout << "Engine warning: " << tokens.at(1u) << '\n';
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

void MainWindow::log_move(const board::CheckersBoard::Move& move) {
    const auto label {std::to_string(++moves) + ". " + board::CheckersBoard::move_to_string(move)};

    szr_moves->Add(new wxStaticText(pnl_moves, wxID_ANY, label));
    szr_moves->AddSpacer(5);
    szr_moves->FitInside(pnl_moves);

    Layout();  // Stupid panels; calling pnl_moves->Layout() was not working; one hour wasted
}

void MainWindow::clear_moves_log() {
    moves = 0u;
    szr_moves->Clear();  // For some stupid reason this is needed as well
    pnl_moves->DestroyChildren();

    Layout();
}
