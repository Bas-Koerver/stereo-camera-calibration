#define NOMINMAX
#ifdef NDEBUG
#include <opencv2/core/utils/logger.hpp>
#endif

#include "utility.hpp"

#include "executors/board_runner.hpp"
#include "executors/recording_runner.hpp"
#include "executors/calibration_runner.hpp"
#include "executors/validation_runner.hpp"

#include <csignal>
#include <CLI/App.hpp>
#include <GLFW/glfw3.h>

int main(int argc, char** argv) {
    // Decrease log level to warning for release builds.
#ifdef NDEBUG
    cv::utils::logging::setLogLevel(cv::utils::logging::LogLevel::LOG_LEVEL_WARNING);
#endif

    // Try to initialise GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW.\n";
        raise(SIGABRT);
    }

    /*
     * CLI
     */
    YACCP::CLI::CliCmdConfig cliCmdConfig;
    YACCP::CLI::CliCmds cliCmds;
    YACCP::CLI::addCli(cliCmdConfig, cliCmds);
    CLI11_PARSE(cliCmds.app, argc, argv);

    /*
    * Global variable declaration
    */
    std::stringstream dateTime;
    dateTime << YACCP::Utility::getCurrentDateTime();
    auto workingDir = std::filesystem::current_path();

    std::filesystem::path path = workingDir / cliCmdConfig.appCmdConfig.userPath;
    auto returnCode{0};


    if (*cliCmds.boardCreationCmd) {
        try {
            YACCP::Executor::runBoardCreation(cliCmdConfig, path, dateTime);
        } catch (const std::exception& err) {
            std::cerr << err.what() << "\n";
        }
    }

    if (*cliCmds.recordingCmd) {
        try {
            returnCode = YACCP::Executor::runRecording(cliCmdConfig, path, dateTime);
        } catch (const std::exception& err) {
            std::cerr << err.what() << "\n";
        }
    }


    if (*cliCmds.validationCmd) {
        try {
            YACCP::Executor::runValidation(cliCmdConfig, path, dateTime);
        } catch (const std::exception& err) {
            std::cerr << err.what() << "\n";
        }
    }

    if (*cliCmds.calibrationCmds.calibration) {
        try {
            YACCP::Executor::runCalibration(cliCmdConfig, cliCmds, path, dateTime);
        } catch (const std::exception& err) {
            std::cerr << err.what() << "\n";
        }
    }

    return returnCode;
}

