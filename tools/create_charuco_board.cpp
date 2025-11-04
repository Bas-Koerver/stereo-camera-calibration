#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/objdetect/charuco_detector.hpp>
#include <opencv2/core/utils/logger.hpp>
#include <CLI/CLI.hpp>

static void createVideo(const cv::Mat& image, cv::Size size, std::string path)
{
    std::string filename = path + ".mp4";
    double fps{60.0};
    int codec{cv::VideoWriter::fourcc('a', 'v', 'c', '1')};

    cv::Mat imageBgr;
    cv::cvtColor(image, imageBgr, cv::COLOR_GRAY2BGR);

    cv::Mat blankImage{cv::Mat::zeros(size, CV_8UC3)};
    cv::VideoWriter writer(filename, codec, fps, size, true);

    if (!writer.isOpened())
    {
        std::cout << "Could not open the output video for write: " << filename << std::endl;
        return;
    }

    for (int i{0}; i < fps * (10 >> 1); i++)
    {
        writer.write(imageBgr);
        writer.write(blankImage);
    }
    writer.release();
}

static void createBoard(int squaresX, int squaresY, int squareLength, int markerLength, int border, int borderPoints,
                        cv::aruco::Dictionary& dictionary, bool generateImage, bool generateVideo, std::string path)
{
    cv::aruco::CharucoBoard board{
        cv::Size(squaresX, squaresY), static_cast<float>(squareLength), static_cast<float>(markerLength), dictionary
    };

    int width{squaresX * squareLength + 2 * border};
    int height{squaresY * squareLength + 2 * border};
    cv::Mat boardImage;
    cv::Size imageSize{width, height};

    board.generateImage(imageSize, boardImage, border, borderPoints);

    if (generateImage) { cv::imwrite((path + ".png"), boardImage); }
    if (generateVideo) { createVideo(boardImage, imageSize, path); }
}

int main(int argc, char** argv)
{
    // Disable OpenCV info messages.
    // cv::utils::logging::setLogLevel(cv::utils::logging::LogLevel::LOG_LEVEL_WARNING);
    CLI::App app{"Generate a ChArUco board image and/or an event video for an event camera."};
    app.get_formatter()->right_column_width(67);

    // Available arguments
    int squaresX{7};
    app.add_option("-x, --width", squaresX, "The amount of squares in the x direction,\n default 7");

    int squaresY{5};
    app.add_option("-y, --height", squaresY, "The amount of squares in the y direction,\n default 5");

    int squareLength{100}; // in pixels
    app.add_option("-s, --square-length", squareLength, "The square length in pixels,\n default 100");

    int markerLength{70}; // in pixels
    app.add_option("-m, --marker-length", markerLength, "The ArUco marker length in pixels,\n default 70");

    int border{squareLength - markerLength};
    app.add_option("-e, --marker-border", border,
                   "The border size (margins) of the ArUco marker in pixels,\n default is square-length - marker-length");

    int borderPoints{1};
    app.add_option("-b, --border-point", borderPoints, "The amount of points (pixels) for the border,\n default 1");

    int dictNumber{8};
    app.add_option("-d, --dictionary", dictNumber,
                   "ArUco dictionary options:\n"
                   "DICT_4X4_50=0, DICT_4X4_100=1, DICT_4X4_250=2, DICT_4X4_1000=3,\n"
                   "DICT_5X5_50=4, DICT_5X5_100=5, DICT_5X5_250=6, DICT_5X5_1000=7,\n"
                   "DICT_6X6_50=8, DICT_6X6_100=9, DICT_6X6_250=10, DICT_6X6_1000=11,\n"
                   "DICT_7X7_50=12, DICT_7X7_100=13, DICT_7X7_250=14, DICT_7X7_1000=15,\n"
                   "DICT_ARUCO_ORIGINAL = 16, default DICT_6X6_50")->expected(0, 16);
    cv::aruco::Dictionary dictionary{cv::aruco::getPredefinedDictionary(dictNumber)};

    std::string path{};
    app.add_option("-p, --path", path, "The path to save the image and/or video to,\n  default is current dir");
    path += "board";

    bool generateImage{false};
    app.add_flag("-i, --image", generateImage,
                 "Whether to generate an image of the generated board,\n default true");
    generateImage = !generateImage;

    bool generateVideo{false};
    app.add_flag("-v, --video", generateVideo,
                 "Whether to generate an event video of the generated board,\n default false");

    CLI11_PARSE(app, argc, argv);

    createBoard(squaresX, squaresY, squareLength, markerLength, border, borderPoints, dictionary, generateImage,
                generateVideo, path);

    return 0;
}
