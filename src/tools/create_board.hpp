#ifndef YACCP_SRC_TOOLS_CREATE_BOARD
#define YACCP_SRC_TOOLS_CREATE_BOARD
#include "../cli/board_creation.hpp"

#include "../config/orchestrator.hpp"

namespace YACCP::CreateBoard {
    void listJobs(const std::filesystem::path& dataPath);

    void charuco(const Config::FileConfig& fileConfig,
                 const CLI::BoardCreationCmdConfig& boardCreationConfig,
                 const std::filesystem::path& jobPath);
}

#endif // YACCP_SRC_TOOLS_CREATE_BOARD
