#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/objdetect/charuco_detector.hpp>

static void generateVideo(const cv::Mat& image, cv::Size size)
{
    double fps = 60.0;
    int codec = cv::VideoWriter::fourcc('M','J','P','G');

    cv::Mat imageBgr;
    cv::cvtColor(image, imageBgr, cv::COLOR_GRAY2BGR);

    cv::Mat blankImage = cv::Mat::zeros(size, CV_8UC4);
    cv::VideoWriter writer("board.avi", cv::CAP_OPENCV_MJPEG, codec, fps, size, true);

    if (!writer.isOpened())
    {
        std::cout  << "Could not open the output video for write: " << std::endl;
        return;
    }

    for (int i = 0; i < fps * (10 >> 1); i++)
    {
        writer.write(imageBgr);
        writer.write(blankImage);
    }
    writer.release();
}

static void createBoard()
{
    int squaresX = 7;
    int squaresY = 5;
    int squareLength = 100; // in pixels
    int markerLength = 70; // in pixels
    int margins = squareLength - markerLength;

    int borderBits = 1;

    cv::aruco::Dictionary dictionary = cv::aruco::getPredefinedDictionary(cv::aruco::DICT_6X6_250);
    cv::aruco::CharucoBoard board(cv::Size(squaresX, squaresY), squareLength, markerLength, dictionary);

    cv::Mat boardImage;
    cv::Size imageSize;
    imageSize.width = squaresX * squareLength + 2 * margins;
    imageSize.height = squaresY * squareLength + 2 * margins;
    board.generateImage(imageSize, boardImage, margins, borderBits);

    cv::imwrite("board.png", boardImage);
    generateVideo(boardImage, imageSize);
}

int main()
{
    createBoard();
}
