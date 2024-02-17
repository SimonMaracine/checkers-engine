#include "common/parameters.hpp"

#include <iostream>  // TODO temporary

#include <wx/spinctrl.h>

namespace parameters {
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

    void ParametersPanel::set_sizer(wxSizer* sizer) {
        SetSizer(sizer);
        szr_parameters = sizer;
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
                std::cout << event.GetValue() << '\n';
            },
            id
        );

        szr_parameter->Add(spn_parameter, 1);

        pnl_parameter->SetSizer(szr_parameter);

        szr_parameters->Add(pnl_parameter, 1);

        Layout();  // Not enough...
    }
}
