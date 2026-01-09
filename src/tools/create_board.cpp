#include "create_board.hpp"

#include <iostream>

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/objdetect/charuco_detector.hpp>

namespace YACCP {
    void CreateBoard::generateVideo(const cv::Mat& image, cv::Size size, std::filesystem::path jobPath) {
        const std::string filename = (jobPath / "charuco_board.mp4").string();
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

    void CreateBoard::charuco(const Config::FileConfig& fileConfig,
                              const CLI::BoardCreationCmdConfig& boardCreationConfig,
                              const std::filesystem::path& jobPath) {
        const cv::aruco::Dictionary dictionary{cv::aruco::getPredefinedDictionary(fileConfig.detectionConfig.openCvDictionaryId)};
        const cv::aruco::CharucoBoard board{
            fileConfig.boardConfig.boardSize,
            static_cast<float>(boardCreationConfig.squareLength),
            static_cast<float>(boardCreationConfig.markerLength),
            dictionary
        };

        const int width{
            fileConfig.boardConfig.boardSize.width * boardCreationConfig.squareLength + 2 * boardCreationConfig.border
        };
        const int height{
            fileConfig.boardConfig.boardSize.height * boardCreationConfig.squareLength + 2 * boardCreationConfig.border
        };
        cv::Mat boardImage;
        const cv::Size imageSize(width, height);

        board.generateImage(imageSize, boardImage, boardCreationConfig.border, boardCreationConfig.borderPoints);

        if (boardCreationConfig.generateImage) cv::imwrite((jobPath / "charuco_board.png").string(), boardImage);

        if (boardCreationConfig.generateVideo) generateVideo(boardImage, imageSize, jobPath);
    }
} // namespace YACCP
