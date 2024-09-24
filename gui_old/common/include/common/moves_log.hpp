#pragma once

#include <common/wx.hpp>
#include "common/board.hpp"

namespace moves_log {
    class MovesLog : public wxScrolledWindow {
    public:
        explicit MovesLog(wxWindow* parent);

        void log_move(const board::CheckersBoard::Move& move);
        void clear_log();

        void set_sizer(wxSizer* sizer);
    private:
        wxSizer* m_sizer {nullptr};

        unsigned int m_move_count {0};
    };
}
