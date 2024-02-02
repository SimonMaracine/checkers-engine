#include "main_window.hpp"

#include <iostream>
#include <cassert>
#include <cstdio>

#include <wx/statline.h>

#include "fen_string_dialog.hpp"

static constexpr int START_ENGINE {10};
static constexpr int RESET_BOARD {11};
static constexpr int SET_POSITION {12};
static constexpr int BLACK {13};
static constexpr int WHITE {14};

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
    board = new CheckersBoard(this, -1, -1, 400,
        [this](const CheckersBoard::Move& move) {
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
    szr_right_side->Add(new wxStaticLine(pnl_right_side), 1);  // FIXME
    szr_right_side->AddSpacer(10);

    wxPanel* pnl_players {new wxPanel(pnl_right_side)};

    wxBoxSizer* szr_players {new wxBoxSizer(wxHORIZONTAL)};

    wxPanel* pnl_black {new wxPanel(pnl_players)};

    wxBoxSizer* szr_black {new wxBoxSizer(wxVERTICAL)};

    szr_black->Add(new wxStaticText(pnl_black, wxID_ANY, "Black"));

    btn_black_human = new wxRadioButton(pnl_black, BLACK, "Human");
    szr_black->Add(btn_black_human);

    btn_black_computer = new wxRadioButton(pnl_black, BLACK, "Computer");
    szr_black->Add(btn_black_computer);

    pnl_black->SetSizer(szr_black);

    wxPanel* pnl_white {new wxPanel(pnl_players)};

    wxBoxSizer* szr_white {new wxBoxSizer(wxVERTICAL)};

    szr_white->Add(new wxStaticText(pnl_white, wxID_ANY, "White"));

    btn_white_human = new wxRadioButton(pnl_white, WHITE, "Human");
    szr_white->Add(btn_white_human);

    btn_white_computer = new wxRadioButton(pnl_white, WHITE, "Computer");
    btn_white_computer->SetValue(true);
    szr_white->Add(btn_white_computer);

    pnl_white->SetSizer(szr_white);

    szr_players->Add(pnl_black);
    szr_players->Add(pnl_white);

    pnl_players->SetSizer(szr_players);

    szr_right_side->Add(pnl_players);

    szr_right_side->AddSpacer(10);
    szr_right_side->Add(new wxStaticLine(pnl_right_side), 1);  // FIXME
    szr_right_side->AddSpacer(10);

    txt_engine = new wxStaticText(pnl_right_side, wxID_ANY, ENGINE);
    szr_right_side->Add(txt_engine);

    pnl_right_side->SetSizer(szr_right_side);

    wxBoxSizer* szr_main {new wxBoxSizer(wxHORIZONTAL)};

    szr_main->Add(board, 1, wxEXPAND | wxALL);
    szr_main->AddSpacer(20);
    szr_main->Add(pnl_right_side, 1);

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

        txt_engine->SetLabelText(ENGINE + dialog.GetName());

        engine = std::make_unique<Engine>();
        engine->start(dialog.GetPath().ToStdString(), [this](const auto& message) { on_engine_message(message); });
    }
}

void MainWindow::on_reset_board(wxCommandEvent&) {
    board->reset();
    game.txt_status->SetLabelText(STATUS + "game in progress");
    game.txt_player->SetLabelText(PLAYER + "black");
    game.txt_plies_without_advancement->SetLabelText(PLIES_WITHOUT_ADVANCEMENT + "0");
    game.txt_repetition_size->SetLabelText(REPETITION_SIZE + "0");
}

void MainWindow::on_set_position(wxCommandEvent&) {
    FenStringDialog dialog {this, wxID_ANY};

	if (dialog.ShowModal() == wxID_OK) {
        board->set_position(dialog.get_fen_string().ToStdString());
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
    } else {
        assert(btn_black_computer->GetValue());
        black = Player::Computer;
    }
}

void MainWindow::on_white_change(wxCommandEvent&) {
    if (btn_white_human->GetValue()) {
        assert(!btn_white_computer->GetValue());
        white = Player::Human;
    } else {
        assert(btn_white_computer->GetValue());
        white = Player::Computer;
    }
}

void MainWindow::on_piece_move(const CheckersBoard::Move& move) {
    switch (move.type) {
        case CheckersBoard::MoveType::Normal:
            std::cout << move.normal.source_index << " -> " << move.normal.destination_index << '\n';
            break;
        case CheckersBoard::MoveType::Capture:
            std::cout << move.capture.source_index << " -> " << move.capture.destination_indices[move.capture.destination_indices_size - 1u] << '\n';
            break;
    }

    game.txt_status->SetLabelText(STATUS + game_over_text());
    game.txt_player->SetLabelText(PLAYER + (board->get_player() == CheckersBoard::Player::Black ? "black" : "white"));
    game.txt_plies_without_advancement->SetLabelText(PLIES_WITHOUT_ADVANCEMENT + wxString::Format("%u", board->get_plies_without_advancement()));
    game.txt_repetition_size->SetLabelText(REPETITION_SIZE + wxString::Format("%zu", board->get_repetition_size()));

    update_board_user_input();
}

void MainWindow::on_engine_message(const std::string& message) {
    std::cout << "ENGINE MESSAGE: " << message;
}

int MainWindow::get_ideal_board_size() {
    const wxSize size {board->GetSize()};

    return std::min(size.GetHeight(), size.GetWidth());
}

const char* MainWindow::game_over_text() {
    switch (board->get_game_over()) {
        case CheckersBoard::GameOver::None:
            return "game in progress";
        case CheckersBoard::GameOver::WinnerBlack:
            return "game over (winner black)";
        case CheckersBoard::GameOver::WinnerWhite:
            return "game over (winner white)";
        case CheckersBoard::GameOver::Tie:
            return "game over (tie)";
    }

    return nullptr;
}

void MainWindow::update_board_user_input() {
    const Player PLAYERS[2u] { black, white };

    if (PLAYERS[static_cast<unsigned int>(board->get_player()) - 1u] == Player::Computer) {
        board->set_user_input(false);
    } else {
        board->set_user_input(true);
    }
}
