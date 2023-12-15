#include <iostream>

#include <wx/wx.h>
#include <wx/statline.h>

#include "main_window.hpp"
#include "board.hpp"
#include "fen_string_dialog.hpp"

static constexpr int RESET_BOARD {10};
static constexpr int SET_POSITION {11};

static const wxString STATUS {"Status: "};
static const wxString PLAYER {"Player: "};
static const wxString PLIES_WITHOUT_ADVANCEMENT {"Plies without advancement: "};

wxBEGIN_EVENT_TABLE(MainWindow, wxFrame)
    EVT_MENU(RESET_BOARD, MainWindow::on_reset_board)
    EVT_MENU(SET_POSITION, MainWindow::on_set_position)
    EVT_MENU(wxID_EXIT, MainWindow::on_exit)
    EVT_MENU(wxID_ABOUT, MainWindow::on_about)
    EVT_SIZE(MainWindow::on_window_resize)
wxEND_EVENT_TABLE()

MainWindow::MainWindow()
    : wxFrame(nullptr, wxID_ANY, "Checkers Player") {
    setup_menubar();

    SetMinSize(wxSize(896, 504));  // Set minimum size here to trigger a resize event after widgets creation
    setup_widgets();
    Center();
}

MainWindow::~MainWindow() {

}

void MainWindow::setup_menubar() {
    wxMenu* men_file {new wxMenu};
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
    board = new Board(this, -1, -1, 400,
        [this](const Board::Move& move) {
            return on_piece_move(move);
        }
    );
    // board->SetBackgroundColour(wxColour("blue"));

    right_side = new wxPanel(this);
    // right_side->SetBackgroundColour(wxColour("red"));

    wxBoxSizer* right_side_sizer {new wxBoxSizer(wxVERTICAL)};

    game.status = new wxStaticText(right_side, wxID_ANY, STATUS + "game in progress");
    right_side_sizer->Add(game.status, 1);

    game.player = new wxStaticText(right_side, wxID_ANY, PLAYER + "black");
    right_side_sizer->Add(game.player, 1);

    game.plies_without_advancement = new wxStaticText(right_side, wxID_ANY, PLIES_WITHOUT_ADVANCEMENT + "0");
    right_side_sizer->Add(game.plies_without_advancement, 1);

    right_side->SetSizer(right_side_sizer);

    wxBoxSizer* main_sizer {new wxBoxSizer(wxHORIZONTAL)};

    main_sizer->Add(board, 1, wxEXPAND | wxALL);
    main_sizer->AddSpacer(20);
    main_sizer->Add(right_side, 1);

    SetSizer(main_sizer);
}

void MainWindow::on_exit(wxCommandEvent&) {
    wxExit();
}

void MainWindow::on_reset_board(wxCommandEvent&) {
    board->reset();
    game.status->SetLabelText(STATUS + "game in progress");
    game.player->SetLabelText(PLAYER + "black");
    game.plies_without_advancement->SetLabelText(PLIES_WITHOUT_ADVANCEMENT + "0");
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

void MainWindow::on_piece_move(const Board::Move& move) {
    switch (move.type) {
        case Board::MoveType::Normal:
            std::cout << move.normal.source_index << " -> " << move.normal.destination_index << '\n';
            break;
        case Board::MoveType::Capture:
            std::cout << move.capture.source_index << " -> " << move.capture.destination_indices[move.capture.destination_indices_size - 1] << '\n';
            break;
    }

    game.status->SetLabelText(STATUS + game_over_text());
    game.player->SetLabelText(PLAYER + (board->get_player() == Board::Player::Black ? "black" : "white"));
    game.plies_without_advancement->SetLabelText(PLIES_WITHOUT_ADVANCEMENT + wxString::Format("%u", board->get_plies_without_advancement()));
}

int MainWindow::get_ideal_board_size() {
    const wxSize size {board->GetSize()};

    return std::min(size.GetHeight(), size.GetWidth());
}

const char* MainWindow::game_over_text() {
    switch (board->get_game_over()) {
        case Board::GameOver::None:
            return "game in progress";
        case Board::GameOver::WinnerBlack:
            return "game over (winner black)";
        case Board::GameOver::WinnerWhite:
            return "game over (winner white)";
        case Board::GameOver::Tie:
            return "game over (tie)";
    }

    return nullptr;
}
