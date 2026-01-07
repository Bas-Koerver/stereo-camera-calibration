#include "viewing.hpp"

#include <stdexcept>

namespace YACCP::Config {
    void parseViewingConfig(const toml::table& tbl, ViewingConfig& config) {
        // [viewing] configuration variables.
        const toml::node_view viewing{tbl["viewing"]};

        config.viewsHorizontal = viewing["views_horizontal"].value_or(3);
    }
} // YACCP::Config