#ifndef YACCP_TOOLS_CREATE_BOARD
#define YACCP_TOOLS_CREATE_BOARD
#include <filesystem>
#include "../cli/board_creation.hpp"
#include "../recoding/job_data.hpp"
#include "../config/orchestrator.hpp"

// Yet Another Camera Calibration Platform.
namespace YACCP {
    class CreateBoard {
    public:
        static void charuco(const Config::FileConfig& fileConfig,
                            const CLI::BoardCreationConfig& boardCreationConfig,
                            const std::filesystem::path& jobPath);

    private:
        static void generateVideo(const cv::Mat& image,
                                  cv::Size size,
                                  std::filesystem::path jobPath);
    };
} // namespace YACCP

#endif // YACCP_TOOLS_CREATE_BOARD
