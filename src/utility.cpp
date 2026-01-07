#include <filesystem>
#include <opencv2/core/mat.hpp>
#include <opencv2/objdetect/aruco_detector.hpp>
#include <opencv2/objdetect/charuco_detector.hpp>

#include "utility.hpp"

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

    std::_Timeobj<char, const tm*> Utility::getCurrentDateTime() {
        // Get the current date and time.
        const auto now = std::chrono::system_clock::now();
        const auto localTime = std::chrono::system_clock::to_time_t(now);

        // Generate timestamp at UTC +0
        return std::put_time(std::gmtime(&localTime), "%F_%H-%M-%S");
    }
} // namespace YACCP