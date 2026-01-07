#include "board.hpp"

#include <stdexcept>

namespace YACCP::Config {
    void parseBoardConfig(const toml::table& tbl, BoardConfig& config) {
        // [board] configuration variables.
        const toml::node_view board{tbl["board"]};
        if (const auto subTbl{board.as_table()}; !subTbl)
            throw std::runtime_error("Missing [board] table");

        config.boardSize.width = board["squares_x"].value_or(7);
        config.boardSize.height = board["squares_y"].value_or(5);

        const auto squareLengthOpt{board["square_length"].value<double>()};
        if (!squareLengthOpt)
            throw std::runtime_error("board.square_length is required");
        config.squareLength = static_cast<float>(*squareLengthOpt);

        const auto markerLengthOpt{board["marker_length"].value<double>()};
        if (!markerLengthOpt)
            throw std::runtime_error("board.marker_length is required");
        config.markerLength = static_cast<float>(*markerLengthOpt);
    }
} // YACCP::Config
