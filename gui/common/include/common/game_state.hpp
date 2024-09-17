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

        wxStaticText* m_txt_status {nullptr};
        wxStaticText* m_txt_player {nullptr};
        wxStaticText* m_txt_plies_without_advancement {nullptr};
        wxStaticText* m_txt_repetition_size {nullptr};
    };
}
