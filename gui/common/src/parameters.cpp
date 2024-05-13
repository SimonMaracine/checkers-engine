#include "common/parameters.hpp"

#include <wx/spinctrl.h>

namespace parameters {
    ParametersPanel::ParametersPanel(wxWindow* parent, int id_start)
        : wxScrolledWindow(parent), id(id_start) {
        FitInside();
        SetScrollRate(0, 10);
    }

    void ParametersPanel::get_engine_parameters(std::vector<std::pair<std::string, std::string>>&& parameters) {
        for (const auto& [name, type] : parameters) {
            if (type == "int") {
                this->parameters[name] = ParameterType::Int;
                engine->getparameter(name);
            }
        }
    }

    void ParametersPanel::add_parameter(const std::string& name, const std::string& value) {
        switch (parameters.at(name)) {
            case ParameterType::Int:
                setup_integer_parameter_widget(name, value, id++);
                break;
        }
    }

    void ParametersPanel::clear_parameters() {
        sizer->Clear();
        DestroyChildren();

        Layout();
    }

    void ParametersPanel::set_sizer(wxSizer* sizer) {
        SetSizer(sizer);
        this->sizer = sizer;
    }

    void ParametersPanel::setup_integer_parameter_widget(const std::string& name, const std::string& value, int id) {
        wxPanel* pnl_parameter {new wxPanel(this)};
        wxBoxSizer* szr_parameter {new wxBoxSizer(wxHORIZONTAL)};

        szr_parameter->Add(new wxStaticText(pnl_parameter, wxID_ANY, name), 1);

        wxSpinCtrl* spn_parameter {new wxSpinCtrl(pnl_parameter, id, value)};

        spn_parameter->Bind(
            wxEVT_SPINCTRL,
            [this, name](wxSpinEvent& event) {
                engine->setparameter(name, std::to_string(event.GetValue()));
            },
            id
        );

        szr_parameter->Add(spn_parameter, 1);

        pnl_parameter->SetSizer(szr_parameter);

        sizer->Add(pnl_parameter);
        sizer->AddSpacer(5);
        sizer->FitInside(this);

        Layout();  // Not enough...
    }
}
