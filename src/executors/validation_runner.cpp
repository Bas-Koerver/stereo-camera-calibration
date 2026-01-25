#include "validation_runner.hpp"

#include "../utility.hpp"

#include "../global_variables/program_defaults.hpp"

#include "../tools/image_validator.hpp"

#include <GLFW/glfw3.h>

namespace YACCP::Executor {
    void runValidation(CLI::CliCmdConfig& cliCmdConfig, std::filesystem::path path, const std::string& dateTime) {
        const std::filesystem::path dataPath{path / "data"};
        if (cliCmdConfig.validationCmdConfig.showAvailableJobs) {
            ImageValidator::listJobs(dataPath);
        }

        // Get the current video mode of the primary monitor.
        // This mode will later be used to retrieve the width and height.
        const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());

        ImageValidator imageValidator;
        imageValidator.validateImages(mode->width - GlobalVariables::windowMargins,
                                      mode->height - GlobalVariables::windowMargins,
                                      dataPath,
                                      cliCmdConfig.validationCmdConfig.jobId);
    }
} // YACCP::Executor