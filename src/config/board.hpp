#ifndef YACCP_CONFIG_BOARD_HPP
#define YACCP_CONFIG_BOARD_HPP
#include <nlohmann/json.hpp>

#include <opencv2/core/types.hpp>

#include <toml++/toml.hpp>

namespace YACCP::Config {
    struct BoardConfig {
        cv::Size boardSize{};
        float squareLength{};
        float markerLength{};
        int squarePixelLength{};
        int markerPixelLength{};
        int marginSize{};
        int borderBits{};
    };

    inline void to_json(nlohmann::json& j, const BoardConfig& b) {
        j = {
            {
                "boardSize",
                {
                    {"width", b.boardSize.width},
                    {"height", b.boardSize.height},
                }
            },
            {"squareLength", b.squareLength},
            {"markerLength", b.markerLength},
            {"squarePixelLength", b.squarePixelLength},
            {"markerPixelLength", b.markerPixelLength},
            {"marginSize", b.marginSize},
            {"borderBits", b.borderBits}
        };
    }

    inline void from_json(const nlohmann::json& j, BoardConfig& b) {
        j.at("boardSize").at("width").get_to(b.boardSize.width);
        j.at("boardSize").at("height").get_to(b.boardSize.height);
        j.at("squareLength").get_to(b.squareLength);
        j.at("markerLength").get_to(b.markerLength);
        j.at("squarePixelLength").get_to(b.squarePixelLength);
        j.at("markerPixelLength").get_to(b.markerPixelLength);
        j.at("marginSize").get_to(b.marginSize);
        j.at("borderBits").get_to(b.borderBits);
    }

    void parseBoardConfig(const toml::table& tbl, BoardConfig& config);
} // YACCP::Config

#endif //YACCP_CONFIG_BOARD_HPP