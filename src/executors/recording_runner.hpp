#ifndef YACCP_SRC_EXECUTOR_RECORDING_RUNNER_HPP
#define YACCP_SRC_EXECUTOR_RECORDING_RUNNER_HPP
#include "../cli/orchestrator.hpp"

namespace YACCP::Executor {
    int runRecording(CLI::CliCmdConfig& cliCmdConfig,
                     const std::filesystem::path& path,
                     const std::string& dateTime);
} // YACCP::Executor

#endif //YACCP_SRC_EXECUTOR_RECORDING_RUNNER_HPP