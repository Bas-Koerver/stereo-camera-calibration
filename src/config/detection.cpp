#include "detection.hpp"

#include "../global_variables/config_defaults.hpp"

// #include <stdexcept>
// #include <opencv2/objdetect/aruco_board.hpp>
// #include <opencv2/objdetect/charuco_detector.hpp>


namespace YACCP::Config {
    void parseDetectionConfig(const toml::table& tbl, DetectionConfig& config) {
        // [detection] configuration variables.
        const auto* detectionTbl{tbl["detection"].as_table()};
        if (!detectionTbl) return;

        config.openCvDictionaryId = (*detectionTbl)["charuco_dictionary"].value_or(GlobalVariables::charucoDictionary);
        config.detectionInterval = (*detectionTbl)["detection_interval"].value_or(GlobalVariables::detectionInterval);
        config.cornerMin = (*detectionTbl)["minimum_corner_fraction"].value_or(GlobalVariables::minCornerFraction);

        // Load custom charuco parameters if set, otherwise load defaults.
        const auto* charuco{(*detectionTbl)["charuco_parameters"].as_table()};
        config.charucoParameters.tryRefineMarkers = (*charuco)["refine"].value_or(
            config.charucoParameters.tryRefineMarkers);
        config.charucoParameters.minMarkers = (*charuco)["min_markers"].value_or(config.charucoParameters.minMarkers);

        // Load custom detector parameters if set, otherwise load defaults.
        const auto* detector{(*detectionTbl)["detector_parameters"].as_table()};
        config.detectorParameters.adaptiveThreshWinSizeMin = (*detector)["adaptiveThreshWinSizeMin"].value_or(
            config.detectorParameters.adaptiveThreshWinSizeMin);
        config.detectorParameters.adaptiveThreshWinSizeMax = (*detector)["adaptiveThreshWinSizeMax"].value_or(
            config.detectorParameters.adaptiveThreshWinSizeMax);
        config.detectorParameters.adaptiveThreshWinSizeStep = (*detector)["adaptiveThreshWinSizeStep"].value_or(
            config.detectorParameters.adaptiveThreshWinSizeStep);
        config.detectorParameters.adaptiveThreshConstant = (*detector)["adaptiveThreshConstant"].value_or(
            config.detectorParameters.adaptiveThreshConstant);
        config.detectorParameters.minMarkerPerimeterRate = (*detector)["minMarkerPerimeterRate"].value_or(
            config.detectorParameters.minMarkerPerimeterRate);
        config.detectorParameters.maxMarkerPerimeterRate = (*detector)["maxMarkerPerimeterRate"].value_or(
            config.detectorParameters.maxMarkerPerimeterRate);
        config.detectorParameters.polygonalApproxAccuracyRate = (*detector)["polygonalApproxAccuracyRate"].value_or(
            config.detectorParameters.polygonalApproxAccuracyRate);
        config.detectorParameters.minCornerDistanceRate = (*detector)["minCornerDistanceRate"].value_or(
            config.detectorParameters.minCornerDistanceRate);
        config.detectorParameters.minDistanceToBorder = (*detector)["minDistanceToBorder"].value_or(
            config.detectorParameters.minDistanceToBorder);
        config.detectorParameters.minMarkerDistanceRate = (*detector)["minMarkerDistanceRate"].value_or(
            config.detectorParameters.minMarkerDistanceRate);
        config.detectorParameters.cornerRefinementMethod = (*detector)["cornerRefinementMethod"].value_or(
            config.detectorParameters.cornerRefinementMethod);
        config.detectorParameters.cornerRefinementWinSize = (*detector)["cornerRefinementWinSize"].value_or(
            config.detectorParameters.cornerRefinementWinSize);
        config.detectorParameters.cornerRefinementMaxIterations = (*detector)["cornerRefinementMaxIterations"].value_or(
            config.detectorParameters.cornerRefinementMaxIterations);
        config.detectorParameters.cornerRefinementMinAccuracy = (*detector)["cornerRefinementMinAccuracy"].value_or(
            config.detectorParameters.cornerRefinementMinAccuracy);
        config.detectorParameters.markerBorderBits = (*detector)["markerBorderBits"].value_or(
            config.detectorParameters.markerBorderBits);
        config.detectorParameters.perspectiveRemovePixelPerCell = (*detector)["perspectiveRemovePixelPerCell"].value_or(
            config.detectorParameters.perspectiveRemovePixelPerCell);
        config.detectorParameters.perspectiveRemoveIgnoredMarginPerCell = (*detector)[
            "perspectiveRemoveIgnoredMarginPerCell"].value_or(
            config.detectorParameters.perspectiveRemoveIgnoredMarginPerCell);
        config.detectorParameters.maxErroneousBitsInBorderRate = (*detector)["maxErroneousBitsInBorderRate"].value_or(
            config.detectorParameters.maxErroneousBitsInBorderRate);
        config.detectorParameters.minOtsuStdDev = (*detector)["minOtsuStdDev"].value_or(
            config.detectorParameters.minOtsuStdDev);
        config.detectorParameters.errorCorrectionRate = (*detector)["errorCorrectionRate"].value_or(
            config.detectorParameters.errorCorrectionRate);
        config.detectorParameters.aprilTagQuadDecimate = (*detector)["aprilTagQuadDecimate"].value_or(
            config.detectorParameters.aprilTagQuadDecimate);
        config.detectorParameters.aprilTagQuadSigma = (*detector)["aprilTagQuadSigma"].value_or(
            config.detectorParameters.aprilTagQuadSigma);
        config.detectorParameters.aprilTagMinClusterPixels = (*detector)["aprilTagMinClusterPixels"].value_or(
            config.detectorParameters.aprilTagMinClusterPixels);
        config.detectorParameters.aprilTagMaxNmaxima = (*detector)["aprilTagMaxNmaxima"].value_or(
            config.detectorParameters.aprilTagMaxNmaxima);
        config.detectorParameters.aprilTagCriticalRad = (*detector)["aprilTagCriticalRad"].value_or(
            config.detectorParameters.aprilTagCriticalRad);
        config.detectorParameters.aprilTagMaxLineFitMse = (*detector)["aprilTagMaxLineFitMse"].value_or(
            config.detectorParameters.aprilTagMaxLineFitMse);
        config.detectorParameters.aprilTagMinWhiteBlackDiff = (*detector)["aprilTagMinWhiteBlackDiff"].value_or(
            config.detectorParameters.aprilTagMinWhiteBlackDiff);
        config.detectorParameters.aprilTagDeglitch = (*detector)["aprilTagDeglitch"].value_or(
            config.detectorParameters.aprilTagDeglitch);
        config.detectorParameters.detectInvertedMarker = (*detector)["detectInvertedMarker"].value_or(
            config.detectorParameters.detectInvertedMarker);
        config.detectorParameters.useAruco3Detection = (*detector)["useAruco3Detection"].value_or(
            config.detectorParameters.useAruco3Detection);
        config.detectorParameters.minSideLengthCanonicalImg = (*detector)["minSideLengthCanonicalImg"].value_or(
            config.detectorParameters.minSideLengthCanonicalImg);
        config.detectorParameters.minMarkerLengthRatioOriginalImg = (*detector)["minMarkerLengthRatioOriginalImg"].
            value_or(config.detectorParameters.minMarkerLengthRatioOriginalImg);
    }
} // YACCP::Config