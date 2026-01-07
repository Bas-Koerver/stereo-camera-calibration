#ifndef YACCP_CONFIG_DETECTION_HPP
#define YACCP_CONFIG_DETECTION_HPP
#include <opencv2/objdetect/charuco_detector.hpp>
#include <toml++/toml.hpp>

namespace YACCP::Config {
    struct DetectionConfig {
        int openCvDictionaryId{};
        int detectionInterval{};
        float cornerMin{};

        cv::aruco::CharucoParameters charucoParameters{};
        cv::aruco::DetectorParameters detectorParameters{};
    };

    void parseDetectionConfig(const toml::table &tbl, DetectionConfig& config);
} // YACCP::Config

#endif //YACCP_CONFIG_DETECTION_HPP
