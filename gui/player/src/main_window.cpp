#include <iostream>

#include <wx/wx.h>

#include "main_window.hpp"
#include "board.hpp"
#include "fen_string_dialog.hpp"

static constexpr int RESET_BOARD = 10;
static constexpr int SET_POSITION = 11;

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

    CreateStatusBar();

    setup_widgets();
    SetMinSize(wxSize(1024, 576));
    Center();
}

MainWindow::~MainWindow() {

}

void MainWindow::setup_menubar() {
    wxMenu* men_file = new wxMenu;
    men_file->Append(RESET_BOARD, "Reset Board");
    men_file->Append(SET_POSITION, "Set Position");
    men_file->Append(wxID_EXIT, "Exit");

    wxMenu* men_help = new wxMenu;
    men_help->Append(wxID_ABOUT, "About");

    wxMenuBar* menu_bar = new wxMenuBar;
    menu_bar->Append(men_file, "File");
    menu_bar->Append(men_help, "Help");

    SetMenuBar(menu_bar);
}

void MainWindow::setup_widgets() {
    board = new Board(this, 20, 20, GetSize().GetHeight() - 120,
        [this](Board::Move move) {
            return on_piece_move(move);
        }
    );
}

void MainWindow::on_exit(wxCommandEvent&) {
    wxExit();
}

void MainWindow::on_reset_board(wxCommandEvent&) {
    board->reset();
}

void MainWindow::on_set_position(wxCommandEvent& event) {
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
        board->set_board_size(event.GetSize().GetHeight() - 120);
    }
}

bool MainWindow::on_piece_move(const Board::Move& move) {
    switch (move.type) {
        case Board::MoveType::Normal:
            std::cout << move.normal.source_index << " -> " << move.normal.destination_index << '\n';
            break;
        case Board::MoveType::Capture:
            std::cout << move.capture.source_index << " -> " << move.capture.destination_index << '\n';
            break;
    }

    return true;
}
