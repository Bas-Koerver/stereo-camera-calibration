#ifndef YACCP_CONFIG_BOARD_HPP
#define YACCP_CONFIG_BOARD_HPP
#include <opencv2/core/types.hpp>
#include <toml++/toml.hpp>

namespace YACCP::Config {
    struct BoardConfig {
        cv::Size boardSize{};
        float squareLength{};
        float markerLength{};
    };

    void parseBoardConfig(const toml::table& tbl, BoardConfig& config);
} // YACCP::Config

#endif //YACCP_CONFIG_BOARD_HPP
