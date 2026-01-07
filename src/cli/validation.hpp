#ifndef YACCP_CLI_VALIDATION_HPP
#define YACCP_CLI_VALIDATION_HPP
#include <CLI/App.hpp>

namespace YACCP::CLI
{
    struct ValidationConfig
    {
        bool showAvailableJobs{};
        std::string jobId{};
    };

    ::CLI::App* addValidationCmd(::CLI::App &app, ValidationConfig &config);
} // namespace YACCP::CLI

#endif // YACCP_CLI_VALIDATION_HPP
