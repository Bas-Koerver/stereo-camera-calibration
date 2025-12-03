#include <filesystem>
#include <opencv2/core/mat.hpp>
#include <opencv2/objdetect/aruco_detector.hpp>
#include <opencv2/objdetect/charuco_detector.hpp>

#include "utility.hpp"

namespace YACC {
    bool utility::createDirs(const std::string &path) {
        std::filesystem::path p(path);
        return std::filesystem::create_directories(p);
    }

    // MISRA deviation: OpenCV Charuco API requires std::vector
    std::tuple<
        bool,
        std::vector<int>,
        std::vector<std::vector<cv::Point2f> >,
        std::vector<int>,
        std::vector<cv::Point2f>
    > utility::findBoard(const cv::aruco::CharucoDetector &charucoDector,
                         cv::Mat &gray,
                         int cornerMin) {

        bool boardFound = false;
        std::vector<int> markerIds;
        std::vector<std::vector<cv::Point2f> > markerCorners;
        std::vector<int> chafrucoIds;
        std::vector<cv::Point2f> charucoCorners;

        charucoDector.detectBoard(gray, charucoCorners, chafrucoIds, markerCorners, markerIds);

        if (charucoCorners.size() > cornerMin) {
            boardFound = true;
        }
        return {boardFound, markerIds, markerCorners, chafrucoIds, charucoCorners};
    }
}
