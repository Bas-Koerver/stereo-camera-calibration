#ifndef YACCP_SRC_CLI_BOARD_CREATION_HPP
#define YACCP_SRC_CLI_BOARD_CREATION_HPP
#include "../global_variables/cli_defaults.hpp"

#include <CLI/App.hpp>


namespace YACCP::CLI {
    struct BoardCreationCmdConfig {
        bool showAvailableJobs{};
        std::string jobId{};
        bool generateImage{GlobalVariables::generateImage};
        bool generateVideo{GlobalVariables::generateVideo};
    };

    ::CLI::App* addBoardCreationCmd(::CLI::App & app, BoardCreationCmdConfig & config);
} // namespace YACCP::CLI

#endif // YACCP_SRC_CLI_BOARD_CREATION_HPP