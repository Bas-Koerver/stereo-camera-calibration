#ifndef YACCP_SRC_CLI_RECORDING_HPP
#define YACCP_SRC_CLI_RECORDING_HPP
#include <CLI/App.hpp>

namespace YACCP::CLI {
    struct RecordingCmdConfig {
        bool showAvailableJobs{};
        std::string jobId{};
        bool showAvailableCams{};
    };

    ::CLI::App* addRecordingCmd(::CLI::App & app, RecordingCmdConfig & config);
} // namespace YACCP::CLI

#endif // YACCP_SRC_CLI_RECORDING_HPP