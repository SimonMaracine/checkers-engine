#include <list>
#include <iostream>

#include <wx/wx.h>

#include "main_window.hpp"
#include "board.hpp"

wxBEGIN_EVENT_TABLE(MainWindow, wxFrame)
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
        [this](const Board::Piece& piece, Board::Square target, const std::list<Board::Square>& targets) {
            return on_piece_move(piece, target, targets);
        }
    );
}

void MainWindow::on_exit(wxCommandEvent& event) {
    wxExit();
}

void MainWindow::on_about(wxCommandEvent& event) {
    wxMessageBox(
        "Checkers Player, an implementation of the game of checkers.",
        "About",
        wxOK | wxICON_INFORMATION
    );
}

void MainWindow::on_window_resize(wxSizeEvent& event) {
    board->set_size(event.GetSize().GetHeight() - 120);
}

bool MainWindow::on_piece_move(const Board::Piece& piece, Board::Square target, const std::list<Board::Square>& targets) {
    std::cout << piece.square.file << ", " << piece.square.rank << " -> " << target.file << ", " << target.rank << '\n';

    return true;
}
