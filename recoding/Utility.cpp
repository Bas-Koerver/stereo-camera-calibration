#include <filesystem>
#include <opencv2/core/mat.hpp>
#include <opencv2/objdetect/aruco_detector.hpp>
#include <opencv2/objdetect/charuco_detector.hpp>

#include "Utility.hpp"

namespace YACCP {
    // MISRA deviation: OpenCV Charuco API requires std::vector
    CharucoResults Utility::findBoard(const cv::aruco::CharucoDetector &charucoDetector,
                                      const cv::Mat &gray,
                                      const int cornerMin) {
        CharucoResults charucoResults;

        charucoDetector.detectBoard(gray, charucoResults.charucoCorners, charucoResults.charucoIds,
                                    charucoResults.markerCorners,
                                    charucoResults.markerIds);

        if (charucoResults.charucoCorners.size() > cornerMin) {
            charucoResults.boardFound = true;
        }
        return charucoResults;
    }
} // namespace YACCP