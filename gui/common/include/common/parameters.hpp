#pragma once

#include <unordered_map>
#include <string>
#include <utility>
#include <vector>

#include <wx/wx.h>

#include "common/engine.hpp"

namespace parameters {
    class ParametersPanel : public wxScrolledWindow {
    public:
        ParametersPanel(wxWindow* parent, int id_start)
            : wxScrolledWindow(parent), id(id_start) {
            FitInside();
            SetScrollRate(0, 10);
        }

        void get_engine_parameters(std::vector<std::pair<std::string, std::string>>&& parameters);
        void add_parameter(const std::string& name, const std::string& value);

        void set_sizer(wxSizer* sizer);
        void set_engine(engine::Engine* engine) { this->engine = engine; }
    private:
        void setup_integer_parameter_widget(const std::string& name, const std::string& value, int id);

        enum class ParameterType {
            Int
        };

        std::unordered_map<std::string, ParameterType> parameters;

        wxSizer* szr_parameters {nullptr};
        engine::Engine* engine {nullptr};
        int id {};
    };
}
