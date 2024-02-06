#include "main_window.hpp"

#include <iostream>
#include <cassert>
#include <cstdio>
#include <cstring>
#include <algorithm>

#include <wx/statline.h>

#include "fen_string_dialog.hpp"

enum {
    START_ENGINE = 10,
    RESET_BOARD,
    SET_POSITION,
    BLACK,
    WHITE
};

static const wxString STATUS {"Status: "};
static const wxString PLAYER {"Player: "};
static const wxString PLIES_WITHOUT_ADVANCEMENT {"Plies without advancement: "};
static const wxString REPETITION_SIZE {"Repetition size: "};
static const wxString ENGINE {"Engine: "};

wxBEGIN_EVENT_TABLE(MainWindow, wxFrame)
    EVT_MENU(START_ENGINE, MainWindow::on_start_engine)
    EVT_MENU(RESET_BOARD, MainWindow::on_reset_board)
    EVT_MENU(SET_POSITION, MainWindow::on_set_position)
    EVT_MENU(wxID_EXIT, MainWindow::on_exit)
    EVT_MENU(wxID_ABOUT, MainWindow::on_about)
    EVT_SIZE(MainWindow::on_window_resize)
    EVT_RADIOBUTTON(BLACK, MainWindow::on_black_change)
    EVT_RADIOBUTTON(WHITE, MainWindow::on_white_change)
    EVT_CLOSE(MainWindow::on_close)
wxEND_EVENT_TABLE()

MainWindow::MainWindow()
    : wxFrame(nullptr, wxID_ANY, "Checkers Player") {
    setup_menubar();

    SetMinSize(wxSize(896, 504));  // Set minimum size here to trigger a resize event after widgets creation
    setup_widgets();
    Center();
}

void MainWindow::setup_menubar() {
    wxMenu* men_file {new wxMenu};
    men_file->Append(START_ENGINE, "Start Engine");
    men_file->Append(RESET_BOARD, "Reset Board");
    men_file->Append(SET_POSITION, "Set Position");
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

    game.txt_status = new wxStaticText(pnl_right_side, wxID_ANY, STATUS + "game in progress");
    szr_right_side->Add(game.txt_status, 1);

    game.txt_player = new wxStaticText(pnl_right_side, wxID_ANY, PLAYER + "black");
    szr_right_side->Add(game.txt_player, 1);

    game.txt_plies_without_advancement = new wxStaticText(pnl_right_side, wxID_ANY, PLIES_WITHOUT_ADVANCEMENT + "0");
    szr_right_side->Add(game.txt_plies_without_advancement, 1);

    game.txt_repetition_size = new wxStaticText(pnl_right_side, wxID_ANY, REPETITION_SIZE + "0");
    szr_right_side->Add(game.txt_repetition_size, 1);

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

    txt_engine = new wxStaticText(pnl_right_side, wxID_ANY, ENGINE);
    szr_right_side->Add(txt_engine, 1);

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
    if (engine != nullptr) {
        engine->stop();
    }

    wxExit();
}

void MainWindow::on_start_engine(wxCommandEvent&) {
    wxFileDialog dialog {this};

    if (dialog.ShowModal() == wxID_OK) {
        if (engine != nullptr) {
            engine->stop();
        }

        txt_engine->SetLabelText(ENGINE + dialog.GetFilename());

        engine = std::make_unique<engine::Engine>([this](const auto& message) { on_engine_message(message); });

        try {
            engine->start(dialog.GetPath().ToStdString());
        } catch (int) {
            std::cout << "Error creating process\n";
        }
    }
}

void MainWindow::on_reset_board(wxCommandEvent&) {
    board->reset();
    game.txt_status->SetLabelText(STATUS + "game in progress");
    game.txt_player->SetLabelText(PLAYER + "black");
    game.txt_plies_without_advancement->SetLabelText(PLIES_WITHOUT_ADVANCEMENT + "0");
    game.txt_repetition_size->SetLabelText(REPETITION_SIZE + "0");

    if (engine != nullptr) {
        engine->newgame();
    }

    clear_moves_log();
}

void MainWindow::on_set_position(wxCommandEvent&) {
    FenStringDialog dialog {this, wxID_ANY};

	if (dialog.ShowModal() == wxID_OK) {
        board->set_position(dialog.get_fen_string().ToStdString());
        game.txt_player->SetLabelText(PLAYER + (board->get_player() == board::CheckersBoard::Player::Black ? "black" : "white"));
    }
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
        Layout();  // Force layout refresh
        board->set_board_size(get_ideal_board_size());
    }

    event.Skip();
}

void MainWindow::on_black_change(wxCommandEvent&) {
    if (btn_black_human->GetValue()) {
        assert(!btn_black_computer->GetValue());
        black = Player::Human;

        if (board->get_player() == board::CheckersBoard::Player::Black) {
            board->set_user_input(true);
        }
    } else {
        assert(btn_black_computer->GetValue());
        black = Player::Computer;

        if (board->get_player() == board::CheckersBoard::Player::Black) {
            board->set_user_input(false);
        }
    }
}

void MainWindow::on_white_change(wxCommandEvent&) {
    if (btn_white_human->GetValue()) {
        assert(!btn_white_computer->GetValue());
        white = Player::Human;

        if (board->get_player() == board::CheckersBoard::Player::White) {
            board->set_user_input(true);
        }
    } else {
        assert(btn_white_computer->GetValue());
        white = Player::Computer;

        if (board->get_player() == board::CheckersBoard::Player::White) {
            board->set_user_input(false);
        }
    }
}

void MainWindow::on_close(wxCloseEvent&) {
    if (engine != nullptr) {
        engine->stop();
    }

    wxExit();
}

void MainWindow::on_piece_move(const board::CheckersBoard::Move& move) {
    const auto label {std::to_string(++moves) + ". " + board::CheckersBoard::move_to_string(move)};

    szr_moves->Add(new wxStaticText(pnl_moves, wxID_ANY, label));
    szr_moves->AddSpacer(5);
    Layout();  // Stupid panels; calling pnl_moves->Layout() was not working; one hour wasted

    game.txt_status->SetLabelText(STATUS + game_over_text());
    game.txt_player->SetLabelText(PLAYER + (board->get_player() == board::CheckersBoard::Player::Black ? "black" : "white"));
    game.txt_plies_without_advancement->SetLabelText(PLIES_WITHOUT_ADVANCEMENT + wxString::Format("%u", board->get_plies_without_advancement()));
    game.txt_repetition_size->SetLabelText(REPETITION_SIZE + wxString::Format("%zu", board->get_repetition_size()));

    if (update_board_user_input() == Player::Human) {
        if (engine != nullptr) {
            engine->move(board::CheckersBoard::move_to_string(move));
            engine->go();
        }
    }
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

MainWindow::Player MainWindow::update_board_user_input() {
    const Player PLAYERS[2u] { black, white };

    if (PLAYERS[static_cast<unsigned int>(board->get_player()) - 1u] == Player::Computer) {
        board->set_user_input(false);

        return Player::Human;
    } else {
        board->set_user_input(true);

        return Player::Computer;
    }
}

void MainWindow::process_engine_message(const std::string& message) {
    const auto tokens {parse_message(message)};

    if (tokens.empty()) {
        return;
    }

    if (tokens.at(0u) == "WARNING") {
        if (tokens.size() > 1u) {
            std::cout << "Engine warning: " << tokens.at(1u) << '\n';
        }
    } else if (tokens.at(0u) == "BESTMOVE") {
        if (tokens.size() > 1u) {
            const auto& move_string {tokens.at(1u)};

            board->play_move(move_string.substr(0u, move_string.size() - 1u));
        }
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

void MainWindow::clear_moves_log() {
    szr_moves->Clear();  // For some stupid reason this is needed as well
    pnl_moves->DestroyChildren();
    Layout();
    moves = 0u;
}
