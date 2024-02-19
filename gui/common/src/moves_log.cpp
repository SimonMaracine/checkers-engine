#include "common/moves_log.hpp"

namespace moves_log {
    MovesLog::MovesLog(wxWindow* parent)
        : wxScrolledWindow(parent) {
        FitInside();
        SetScrollRate(0, 10);
    }

    void MovesLog::log_move(const board::CheckersBoard::Move& move) {
        const auto label {std::to_string(++move_count) + ". " + board::CheckersBoard::move_to_string(move)};

        sizer->Add(new wxStaticText(this, wxID_ANY, label));
        sizer->AddSpacer(5);
        sizer->FitInside(this);

        Layout();  // Stupid panels; calling Layout() is not working; one hour wasted
    }

    void MovesLog::clear_log() {
        move_count = 0u;
        sizer->Clear();  // For some stupid reason this is needed as well
        DestroyChildren();

        Layout();
    }

    void MovesLog::set_sizer(wxSizer* sizer) {
        SetSizer(sizer);
        this->sizer = sizer;
    }
}
