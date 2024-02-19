#pragma once

#include <wx/wx.h>

#include "common/board.hpp"

namespace moves_log {
    class MovesLog : public wxScrolledWindow {
    public:
        MovesLog(wxWindow* parent);

        void log_move(const board::CheckersBoard::Move& move);
        void clear_log();

        void set_sizer(wxSizer* sizer);
    private:
        wxSizer* sizer {nullptr};

        unsigned int move_count {0u};
    };
}
