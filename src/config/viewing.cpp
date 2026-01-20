#include "viewing.hpp"

#include "../global_variables/config_defaults.hpp"

// #include <stdexcept>


namespace YACCP::Config {
    void parseViewingConfig(const toml::table& tbl, ViewingConfig& config) {
        // [viewing] configuration variables.
        const toml::node_view viewing{tbl["viewing"]};

        config.viewsHorizontal = viewing["views_horizontal"].value_or(GlobalVariables::camViewsHorizontal);
    }
} // YACCP::Config