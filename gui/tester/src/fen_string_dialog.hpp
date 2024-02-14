#pragma once

#include <wx/wx.h>

class FenStringDialog : public wxDialog {
public:
	FenStringDialog(wxWindow* parent, wxWindowID id);

	wxString get_fen_string() const;
private:
	wxTextCtrl* txt_fen_string {nullptr};
};
