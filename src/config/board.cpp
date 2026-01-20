#include "board.hpp"

#include "orchestrator.hpp"

#include "../global_variables/config_defaults.hpp"

// #include <stdexcept>


namespace YACCP::Config {
    void parseBoardConfig(const toml::table& tbl, BoardConfig& config) {
        // [board] configuration variables.
        const auto* boardTbl{tbl["board"].as_table()};
        if (!boardTbl)
            throw std::runtime_error("Missing [board] table");

        config.boardSize.width = (*boardTbl)["squares_x"].value_or(GlobalVariables::boardWidth);
        config.boardSize.height = (*boardTbl)["squares_y"].value_or(GlobalVariables::boardHeight);

        config.squarePixelLength = (*boardTbl)["square_pixel_length"].value_or(GlobalVariables::squarePixelLength);
        config.markerPixelLength = (*boardTbl)["marker_pixel_length"].value_or(GlobalVariables::markerPixelLength);
        config.marginSize = (*boardTbl)["margin_size"].value_or(config.squarePixelLength - config.markerPixelLength);
        config.borderBits = (*boardTbl)["border_bits"].value_or(GlobalVariables::borderBits);

        config.squareLength = requireVariable<float>(*boardTbl, "square_length", "board");
        config.markerLength = requireVariable<float>(*boardTbl, "marker_length", "board");
    }
} // YACCP::Config