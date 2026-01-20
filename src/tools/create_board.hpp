#ifndef YACCP_TOOLS_CREATE_BOARD
#define YACCP_TOOLS_CREATE_BOARD
#include "../cli/board_creation.hpp"

#include "../config/orchestrator.hpp"

// #include <filesystem>

// Yet Another Camera Calibration Platform.
namespace YACCP {
    class CreateBoard {
    public:
        static void listJobs(const std::filesystem::path& dataPath);

        // TODO: Add different board types (chessboard, circle grid).
        static void charuco(const Config::FileConfig& fileConfig,
                            const CLI::BoardCreationCmdConfig& boardCreationConfig,
                            const std::filesystem::path& jobPath);

    private:
        static void generateVideo(const cv::Mat& image,
                                  cv::Size size,
                                  std::filesystem::path jobPath);
    };
} // namespace YACCP

#endif // YACCP_TOOLS_CREATE_BOARD