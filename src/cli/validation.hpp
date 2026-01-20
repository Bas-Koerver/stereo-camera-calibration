#ifndef YACCP_CLI_VALIDATION_HPP
#define YACCP_CLI_VALIDATION_HPP
#include <CLI/App.hpp>

namespace YACCP::CLI {
    struct ValidationCmdConfig {
        bool showAvailableJobs{};
        std::string jobId{};
    };

    ::CLI::App* addValidationCmd(::CLI::App & app, ValidationCmdConfig & config);
} // namespace YACCP::CLI

#endif // YACCP_CLI_VALIDATION_HPP