#include "common/moves_log.hpp"

namespace moves_log {
    MovesLog::MovesLog(wxWindow* parent)
        : wxScrolledWindow(parent) {
        FitInside();
        SetScrollRate(0, 10);
    }

    void MovesLog::log_move(const board::CheckersBoard::Move& move) {
        const auto label {std::to_string(++m_move_count) + ". " + board::CheckersBoard::move_to_string(move)};

        m_sizer->Add(new wxStaticText(this, wxID_ANY, label));
        m_sizer->AddSpacer(5);
        m_sizer->FitInside(this);

        Layout();  // Stupid panels; calling Layout() is not working; one hour wasted
    }

    void MovesLog::clear_log() {
        m_move_count = 0;
        m_sizer->Clear();  // For some stupid reason this is needed as well
        DestroyChildren();

        Layout();
    }

    void MovesLog::set_sizer(wxSizer* sizer) {
        SetSizer(sizer);
        m_sizer = sizer;
    }
}
