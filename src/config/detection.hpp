#ifndef YACCP_SRC_CONFIG_DETECTION_HPP
#define YACCP_SRC_CONFIG_DETECTION_HPP
#include <nlohmann/json.hpp>

#include <opencv2/objdetect/charuco_detector.hpp>

#include <toml++/toml.hpp>

namespace cv::aruco {
    inline void to_json(nlohmann::json& j, const CharucoParameters& p) {
        j = {
            {"minMarkers", p.minMarkers},
            {"tryRefineMarkers", p.tryRefineMarkers}
        };
    }


    inline void from_json(const nlohmann::json& j, CharucoParameters& p) {
        j.at("minMarkers").get_to(p.minMarkers);
        j.at("tryRefineMarkers").get_to(p.tryRefineMarkers);
    }


    inline void to_json(nlohmann::json& j, const DetectorParameters& p) {
        j = {
            {"adaptiveThreshWinSizeMin", p.adaptiveThreshWinSizeMin},
            {"adaptiveThreshWinSizeMax", p.adaptiveThreshWinSizeMax},
            {"adaptiveThreshWinSizeStep", p.adaptiveThreshWinSizeStep},
            {"adaptiveThreshConstant", p.adaptiveThreshConstant},
            {"minMarkerPerimeterRate", p.minMarkerPerimeterRate},
            {"maxMarkerPerimeterRate", p.maxMarkerPerimeterRate},
            {"polygonalApproxAccuracyRate", p.polygonalApproxAccuracyRate},
            {"minCornerDistanceRate", p.minCornerDistanceRate},
            {"minDistanceToBorder", p.minDistanceToBorder},
            {"minMarkerDistanceRate", p.minMarkerDistanceRate},
            {"cornerRefinementMethod", p.cornerRefinementMethod},
            {"cornerRefinementWinSize", p.cornerRefinementWinSize},
            {"cornerRefinementMaxIterations", p.cornerRefinementMaxIterations},
            {"cornerRefinementMinAccuracy", p.cornerRefinementMinAccuracy},
            {"markerBorderBits", p.markerBorderBits},
            {"perspectiveRemovePixelPerCell", p.perspectiveRemovePixelPerCell},
            {"perspectiveRemoveIgnoredMarginPerCell", p.perspectiveRemoveIgnoredMarginPerCell},
            {"maxErroneousBitsInBorderRate", p.maxErroneousBitsInBorderRate},
            {"minOtsuStdDev", p.minOtsuStdDev},
            {"errorCorrectionRate", p.errorCorrectionRate},
            {"aprilTagQuadDecimate", p.aprilTagQuadDecimate},
            {"aprilTagQuadSigma", p.aprilTagQuadSigma},
            {"aprilTagMinClusterPixels", p.aprilTagMinClusterPixels},
            {"aprilTagMaxNmaxima", p.aprilTagMaxNmaxima},
            {"aprilTagCriticalRad", p.aprilTagCriticalRad},
            {"aprilTagMaxLineFitMse", p.aprilTagMaxLineFitMse},
            {"aprilTagMinWhiteBlackDiff", p.aprilTagMinWhiteBlackDiff},
            {"aprilTagDeglitch", p.aprilTagDeglitch},
            {"detectInvertedMarker", p.detectInvertedMarker},
            {"useAruco3Detection", p.useAruco3Detection},
            {"minSideLengthCanonicalImg", p.minSideLengthCanonicalImg},
            {"minMarkerLengthRatioOriginalImg", p.minMarkerLengthRatioOriginalImg}
        };
    }


    inline void from_json(const nlohmann::json& j, DetectorParameters& p) {
        j.at("adaptiveThreshWinSizeMin").get_to(p.adaptiveThreshWinSizeMin);
        j.at("adaptiveThreshWinSizeMax").get_to(p.adaptiveThreshWinSizeMax);
        j.at("adaptiveThreshWinSizeStep").get_to(p.adaptiveThreshWinSizeStep);
        j.at("adaptiveThreshConstant").get_to(p.adaptiveThreshConstant);
        j.at("minMarkerPerimeterRate").get_to(p.minMarkerPerimeterRate);
        j.at("maxMarkerPerimeterRate").get_to(p.maxMarkerPerimeterRate);
        j.at("polygonalApproxAccuracyRate").get_to(p.polygonalApproxAccuracyRate);
        j.at("minCornerDistanceRate").get_to(p.minCornerDistanceRate);
        j.at("minDistanceToBorder").get_to(p.minDistanceToBorder);
        j.at("minMarkerDistanceRate").get_to(p.minMarkerDistanceRate);
        j.at("cornerRefinementMethod").get_to(p.cornerRefinementMethod);
        j.at("cornerRefinementWinSize").get_to(p.cornerRefinementWinSize);
        j.at("cornerRefinementMaxIterations").get_to(p.cornerRefinementMaxIterations);
        j.at("cornerRefinementMinAccuracy").get_to(p.cornerRefinementMinAccuracy);
        j.at("markerBorderBits").get_to(p.markerBorderBits);
        j.at("perspectiveRemovePixelPerCell").get_to(p.perspectiveRemovePixelPerCell);
        j.at("perspectiveRemoveIgnoredMarginPerCell").get_to(p.perspectiveRemoveIgnoredMarginPerCell);
        j.at("maxErroneousBitsInBorderRate").get_to(p.maxErroneousBitsInBorderRate);
        j.at("minOtsuStdDev").get_to(p.minOtsuStdDev);
        j.at("errorCorrectionRate").get_to(p.errorCorrectionRate);
        j.at("aprilTagQuadDecimate").get_to(p.aprilTagQuadDecimate);
        j.at("aprilTagQuadSigma").get_to(p.aprilTagQuadSigma);
        j.at("aprilTagMinClusterPixels").get_to(p.aprilTagMinClusterPixels);
        j.at("aprilTagMaxNmaxima").get_to(p.aprilTagMaxNmaxima);
        j.at("aprilTagCriticalRad").get_to(p.aprilTagCriticalRad);
        j.at("aprilTagMaxLineFitMse").get_to(p.aprilTagMaxLineFitMse);
        j.at("aprilTagMinWhiteBlackDiff").get_to(p.aprilTagMinWhiteBlackDiff);
        j.at("aprilTagDeglitch").get_to(p.aprilTagDeglitch);
        j.at("detectInvertedMarker").get_to(p.detectInvertedMarker);
        j.at("useAruco3Detection").get_to(p.useAruco3Detection);
        j.at("minSideLengthCanonicalImg").get_to(p.minSideLengthCanonicalImg);
        j.at("minMarkerLengthRatioOriginalImg").get_to(p.minMarkerLengthRatioOriginalImg);
    }
}

namespace YACCP::Config {
    struct DetectionConfig {
        int openCvDictionaryId{};
        int detectionInterval{};
        float cornerMin{};

        cv::aruco::CharucoParameters charucoParameters{};
        cv::aruco::DetectorParameters detectorParameters{};
    };


    inline void to_json(nlohmann::json& j, const DetectionConfig& d) {
        j = {
            {"openCvDictionaryId", d.openCvDictionaryId},
            // DetectionInterval is a user variable and not needed to recreate an experiment.
            {"cornerMin", d.cornerMin},
            {"charucoParameters", d.charucoParameters},
            {"detectorParameters", d.detectorParameters},
        };
    }


    inline void from_json(const nlohmann::json& j, DetectionConfig& d) {
        j.at("openCvDictionaryId").get_to(d.openCvDictionaryId);
        j.at("cornerMin").get_to(d.cornerMin);
        j.at("charucoParameters").get_to(d.charucoParameters);
        j.at("detectorParameters").get_to(d.detectorParameters);
    }


    void parseDetectionConfig(const toml::table& tbl, DetectionConfig& config);
} // YACCP::Config

#endif //YACCP_SRC_CONFIG_DETECTION_HPP