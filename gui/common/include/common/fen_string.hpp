#pragma once

#include "common/wx.hpp"

class FenString : public wxDialog {
public:
	FenString(wxWindow* parent, wxWindowID id);

	wxString get_fen_string() const;
private:
	wxTextCtrl* txt_fen_string {nullptr};
};
