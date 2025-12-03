#ifndef STEREO_CAMERA_CALIBRATION_CHARUCO_BOARD_DETECTOR_H
#define STEREO_CAMERA_CALIBRATION_CHARUCO_BOARD_DETECTOR_H

#include <opencv2/objdetect/charuco_detector.hpp>

namespace YACC {
    class utility {
    public:
        static bool createDirs(const std::string &path);

        static std::tuple<
            bool,
            std::vector<int>,
            std::vector<std::vector<cv::Point2f> >,
            std::vector<int>,
            std::vector<cv::Point2f>
        >
        findBoard(const cv::aruco::CharucoDetector &charucoDector, cv::Mat &gray, int cornerMin = 3);
    };
}

#endif //STEREO_CAMERA_CALIBRATION_CHARUCO_BOARD_DETECTOR_H
