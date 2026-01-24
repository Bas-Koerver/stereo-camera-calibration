#ifndef YACCP_SRC_EXECUTOR_BOARD_RUNNER_HPP
#define YACCP_SRC_EXECUTOR_BOARD_RUNNER_HPP
#include "../cli/orchestrator.hpp"

namespace YACCP::Executor {
    void runBoardCreation(const CLI::CliCmdConfig& cliCmdConfig,
                          const std::filesystem::path& path,
                          const std::stringstream& dateTime);
} // YACCP::Executor

#endif //YACCP_SRC_EXECUTOR_BOARD_RUNNER_HPP
