#ifndef YACCP_SRC_CONFIG_VIEWING_HPP
#define YACCP_SRC_CONFIG_VIEWING_HPP
#include <toml++/toml.hpp>

namespace YACCP::Config {
    struct ViewingConfig {
        int viewsHorizontal{};
    };

    void parseViewingConfig(const toml::table& tbl, ViewingConfig& viewingConfig);
} // YACCP::Config

#endif //YACCP_SRC_CONFIG_VIEWING_HPP