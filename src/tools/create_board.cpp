#include "create_board.hpp"

// #include <iostream>

// #include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>
// #include <opencv2/objdetect/charuco_detector.hpp>

namespace YACCP {
    void CreateBoard::generateVideo(const cv::Mat& image, cv::Size size, std::filesystem::path jobPath) {
        const std::string filename = (jobPath / "board.mp4").string();
        constexpr auto fps{60.0};
        const int codec{cv::VideoWriter::fourcc('a', 'v', 'c', '1')};

        cv::Mat imageBgr;
        cv::cvtColor(image, imageBgr, cv::COLOR_GRAY2BGR);

        cv::Mat blankImage{cv::Mat::zeros(size, CV_8UC3)};
        cv::VideoWriter writer(filename, codec, fps, size, true);

        if (!writer.isOpened()) {
            std::cout << "Could not open the output video for write: " << filename << std::endl;
            return;
        }

        for (int i{0}; i < fps * (10 >> 1); i++) {
            writer.write(imageBgr);
            writer.write(blankImage);
        }
        writer.release();
    }

    void CreateBoard::listJobs(const std::filesystem::path& dataPath) {
        std::cout << "jobs missing a board image and/or video: \n";
        for (auto const& entry : std::filesystem::directory_iterator(dataPath)) {
            if (!entry.is_directory()) continue;

            std::filesystem::path rawPath = entry.path() / "images" / "raw";

            const bool jobDataFound{!(entry.path() / "job_data.json").empty()};
            const bool imageFound{!(entry.path() / "board.png").empty()};
            const bool videoFound{!(entry.path() / "video.mp4").empty()};

            if (!jobDataFound || (imageFound && videoFound)) continue;

            std::cout << "  " << entry.path().filename() << "\n";
        }
    }

    void CreateBoard::charuco(const Config::FileConfig& fileConfig,
                              const CLI::BoardCreationCmdConfig& boardCreationConfig,
                              const std::filesystem::path& jobPath) {
        const cv::aruco::Dictionary dictionary{
            cv::aruco::getPredefinedDictionary(fileConfig.detectionConfig.openCvDictionaryId)
        };
        const cv::aruco::CharucoBoard board{
            fileConfig.boardConfig.boardSize,
            static_cast<float>(fileConfig.boardConfig.squarePixelLength),
            static_cast<float>(fileConfig.boardConfig.markerPixelLength),
            dictionary
        };

        const int width{
            fileConfig.boardConfig.boardSize.width * fileConfig.boardConfig.squarePixelLength + 2 * fileConfig.
            boardConfig.marginSize
        };
        const int height{
            fileConfig.boardConfig.boardSize.height * fileConfig.boardConfig.squarePixelLength + 2 * fileConfig.
            boardConfig.marginSize
        };
        cv::Mat boardImage;
        const cv::Size imageSize(width, height);

        board.generateImage(imageSize,
                            boardImage,
                            fileConfig.boardConfig.marginSize,
                            fileConfig.boardConfig.borderBits);

        if (boardCreationConfig.generateImage) cv::imwrite((jobPath / "board.png").string(), boardImage);

        if (boardCreationConfig.generateVideo) generateVideo(boardImage, imageSize, jobPath);
    }
} // namespace YACCP
