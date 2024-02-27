#include "common/game_state.hpp"

namespace game_state {
    static const wxString STATUS {"Status: "};
    static const wxString PLAYER {"Player: "};
    static const wxString PLIES_WITHOUT_ADVANCEMENT {"Plies without advancement: "};
    static const wxString REPETITION_SIZE {"Repetition size: "};
    static const wxString DEFAULT_STATUS {"game not started"};

    GameStatePanel::GameStatePanel(wxWindow* parent, wxSizer* sizer)
        : wxPanel(parent) {
        txt_status = new wxStaticText(this, wxID_ANY, STATUS + DEFAULT_STATUS);
        sizer->Add(txt_status);

        txt_player = new wxStaticText(this, wxID_ANY, PLAYER + "black");
        sizer->Add(txt_player);

        txt_plies_without_advancement = new wxStaticText(this, wxID_ANY, PLIES_WITHOUT_ADVANCEMENT + "0");
        sizer->Add(txt_plies_without_advancement);

        txt_repetition_size = new wxStaticText(this, wxID_ANY, REPETITION_SIZE + "0");
        sizer->Add(txt_repetition_size);
    }

    void GameStatePanel::reset(const board::CheckersBoard* board) {
        txt_status->SetLabelText(STATUS + DEFAULT_STATUS);
        txt_player->SetLabelText(PLAYER + (board->get_player() == board::CheckersBoard::Player::Black ? "black" : "white"));
        txt_plies_without_advancement->SetLabelText(PLIES_WITHOUT_ADVANCEMENT + "0");
        txt_repetition_size->SetLabelText(REPETITION_SIZE + "0");
    }

    void GameStatePanel::update(const board::CheckersBoard* board) {
        txt_status->SetLabelText(STATUS + game_over_text(board->get_game_over()));
        txt_player->SetLabelText(PLAYER + (board->get_player() == board::CheckersBoard::Player::Black ? "black" : "white"));
        txt_plies_without_advancement->SetLabelText(PLIES_WITHOUT_ADVANCEMENT + wxString::Format("%u", board->get_plies_without_advancement()));
        txt_repetition_size->SetLabelText(REPETITION_SIZE + wxString::Format("%zu", board->get_repetition_size()));
    }

    const char* GameStatePanel::game_over_text(board::CheckersBoard::GameOver game_over) {
        switch (game_over) {
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
}
