#include "common/fen_string.hpp"

FenString::FenString(wxWindow* parent, wxWindowID id)
    : wxDialog(parent, id, "FEN String") {
    txt_fen_string = new wxTextCtrl(this, wxID_ANY);

    wxPanel* pnl_buttons {new wxPanel(this)};
    wxBoxSizer* szr_buttons {new wxBoxSizer(wxHORIZONTAL)};

    szr_buttons->Add(new wxButton(pnl_buttons, wxID_OK, "Ok"));
    szr_buttons->AddSpacer(10);
    szr_buttons->Add(new wxButton(pnl_buttons, wxID_CANCEL, "Cancel"));

    pnl_buttons->SetSizer(szr_buttons);

    wxBoxSizer* szr_main {new wxBoxSizer(wxVERTICAL)};

    szr_main->AddStretchSpacer();
    szr_main->Add(txt_fen_string, 0, wxEXPAND | wxLEFT | wxRIGHT);
    szr_main->AddSpacer(10);
    szr_main->Add(pnl_buttons, 1, wxALIGN_CENTER | wxLEFT | wxRIGHT | wxBOTTOM);
    szr_main->AddStretchSpacer();

    SetSizer(szr_main);
}

wxString FenString::get_fen_string() const {
    return txt_fen_string->GetValue();
}
