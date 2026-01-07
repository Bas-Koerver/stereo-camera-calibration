#ifndef YACCP_CLI_BOARD_CREATION_HPP
#define YACCP_CLI_BOARD_CREATION_HPP
#include <CLI/App.hpp>

namespace YACCP::CLI {
    struct BoardCreationConfig
    {
        int squareLength{100};
        int markerLength{70};
        int border{0};
        int borderPoints{1};
        bool generateImage{true};
        bool generateVideo{false};
    };

    ::CLI::App* addBoardCreationCmd(::CLI::App& app, BoardCreationConfig& config);
} // namespace YACCP::CLI

#endif // YACCP_CLI_BOARD_CREATION_HPP
