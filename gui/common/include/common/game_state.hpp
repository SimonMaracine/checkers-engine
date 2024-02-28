#pragma once

#include <common/wx.hpp>
#include "common/board.hpp"

namespace game_state {
    class GameStatePanel : public wxPanel {
    public:
        GameStatePanel(wxWindow* parent, wxSizer* sizer);

        void reset(const board::CheckersBoard* board);
        void update(const board::CheckersBoard* board);
    private:
        const char* game_over_text(board::CheckersBoard::GameOver game_over);

        wxStaticText* txt_status {nullptr};
        wxStaticText* txt_player {nullptr};
        wxStaticText* txt_plies_without_advancement {nullptr};
        wxStaticText* txt_repetition_size {nullptr};
    };
}
