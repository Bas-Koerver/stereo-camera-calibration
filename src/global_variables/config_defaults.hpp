#ifndef YACCP_SRC_GLOBAL_VARIABLES_CONFIG_DEFAULTS_HPP
#define YACCP_SRC_GLOBAL_VARIABLES_CONFIG_DEFAULTS_HPP

namespace YACCP::GlobalVariables {
    // Default [board] variables
    inline constexpr auto boardWidth{7}; // Amount of squares
    inline constexpr auto boardHeight{5};
    inline constexpr auto squarePixelLength{100};
    inline constexpr auto markerPixelLength{70};
    inline constexpr auto borderBits{1};
    inline constexpr auto videoFps{60};

    // Default [detection] variables
    inline constexpr auto charucoDictionary{8};
    inline constexpr auto detectionInterval{2}; // seconds
    inline constexpr auto minCornerFraction{.125F};

    // Default [recording] variables
    inline constexpr auto recordingFps{30};
    inline constexpr auto accumulationTime{33333};

    // Default [view] variables
    inline constexpr auto camViewsHorizontal{3};
}

#endif //YACCP_SRC_GLOBAL_VARIABLES_CONFIG_DEFAULTS_HPP