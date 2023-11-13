#include <wx/wx.h>

#include "fen_string_dialog.hpp"

FenStringDialog::FenStringDialog(wxWindow* parent, wxWindowID id)
    : wxDialog(parent, id, "FEN String") {
    fen_string = new wxTextCtrl(this, wxID_ANY);

    wxPanel* buttons = new wxPanel(this);
    wxBoxSizer* buttons_sizer = new wxBoxSizer(wxHORIZONTAL);

    buttons_sizer->Add(new wxButton(buttons, wxID_OK, "Ok"));
    buttons_sizer->Add(new wxButton(buttons, wxID_CANCEL, "Cancel"));

    buttons->SetSizer(buttons_sizer);

    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);

    sizer->Add(fen_string);
    sizer->AddSpacer(10);
    sizer->Add(buttons);

    SetSizer(sizer);
}

wxString FenStringDialog::get_fen_string() const {
    return fen_string->GetValue();
}
