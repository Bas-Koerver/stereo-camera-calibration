
#include "create_board.hpp"

#include <CLI/CLI.hpp>
#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/core/utils/logger.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/objdetect/charuco_detector.hpp>
#include <opencv2/videoio.hpp>

namespace YACCP {
    namespace CLI {
        struct BoardCreationConfig;
    }

    struct CharucoConfig;

    void CreateBoard::generateVideo(const cv::Mat& image, cv::Size size, std::filesystem::path jobPath) {
        const std::string filename = (jobPath / "charuco_board.mp4").string();
        double fps{60.0};
        int codec{cv::VideoWriter::fourcc('a', 'v', 'c', '1')};

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
                              const CLI::BoardCreationConfig& boardCreationConfig,
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

// int main(int argc, char** argv)
// {
//     // Disable OpenCV info messages.
//     // cv::utils::logging::setLogLevel(cv::utils::logging::LogLevel::LOG_LEVEL_WARNING);
//     CLI::App app{"Generate a ChArUco board image and/or an event video for an event camera."};
//     app.get_formatter()->right_column_width(67);
//
//     // Available arguments
//     int squaresX{7};
//     app.add_option("-x, --width", squaresX, "The amount of squares in the x
//     direction")->default_val(7)->check(CLI::PositiveNumber);
//
//     int squaresY{5};
//     app.add_option("-y, --height", squaresY, "The amount of squares in the y
//     direction")->default_val(5)->check(CLI::PositiveNumber);
//
//     int squareLength{100}; // in pixels
//     app.add_option("-s, --square-length", squareLength, "The square length in
//     pixels")->default_val(100)->check(CLI::PositiveNumber);
//
//     int markerLength{70}; // in pixels
//     app.add_option("-m, --marker-length", markerLength, "The ArUco marker length in
//     pixels")->default_val(70)->check(CLI::PositiveNumber);
//
//     int border{squareLength - markerLength};
//     app.add_option("-e, --marker-border", border,
//                    "The border size (margins) of the ArUco marker in pixels")->default_str("square-length -
//                    marker-length");
//
//     int borderPoints{1};
//     app.add_option("-b, --border-point", borderPoints, "The amount of points (pixels) for the
//     border")->default_val(1);
//
//     int dictNumber{8};
//     app.add_option("-d, --dictionary", dictNumber,
//                    "ArUco dictionary options:\n"
//                    "DICT_4X4_50=0, DICT_4X4_100=1, DICT_4X4_250=2, DICT_4X4_1000=3,\n"
//                    "DICT_5X5_50=4, DICT_5X5_100=5, DICT_5X5_250=6, DICT_5X5_1000=7,\n"
//                    "DICT_6X6_50=8, DICT_6X6_100=9, DICT_6X6_250=10, DICT_6X6_1000=11,\n"
//                    "DICT_7X7_50=12, DICT_7X7_100=13, DICT_7X7_250=14, DICT_7X7_1000=15,\n"
//                    "DICT_ARUCO_ORIGINAL = 16")->default_str("DICT_6X6_50")->check(CLI::Range(0, 16));
//
//     std::string path{};
//     app.add_option("-p, --path", path, "The path to save the image and/or video to")->default_str("current dir");
//     path += "board";
//
//     bool generateImage{false};
//     app.add_flag("-i, --image", generateImage,
//                  "Whether to generate an image of the generated board")->default_str("true");
//     generateImage = !generateImage;
//
//     bool generateVideo{true};
//     app.add_flag("-v, --video", generateVideo,
//                  "Whether to generate an event video of the generated board")->default_str("false");
//
//     CLI11_PARSE(app, argc, argv);
//
//     cv::aruco::Dictionary dictionary{cv::aruco::getPredefinedDictionary(dictNumber)};
//     createBoard(squaresX, squaresY, squareLength, markerLength, border, borderPoints, dictionary, generateImage,
//                 generateVideo, path);
//
//     return 0;
// }
