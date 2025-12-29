#ifndef STEREO_CAMERA_CALIBRATION_CAM_DATA_HPP
#define STEREO_CAMERA_CALIBRATION_CAM_DATA_HPP
#include <readerwriterqueue.h>

#include <nlohmann/json_fwd.hpp>

#include <opencv2/core/mat.hpp>
#include <opencv2/objdetect/aruco_detector.hpp>
#include <opencv2/objdetect/charuco_detector.hpp>

#include "detection_validator.hpp"

namespace cv::aruco {
    inline void to_json(nlohmann::json &j, const CharucoParameters &p) {
        j = {
            {"minMarkers", p.minMarkers},
            {"tryRefineMarkers", p.tryRefineMarkers}
        };
    }

    inline void from_json(const nlohmann::json &j, CharucoParameters &p) {
        j.at("minMarkers").get_to(p.minMarkers);
        j.at("tryRefineMarkers").get_to(p.tryRefineMarkers);
    }

    inline void to_json(nlohmann::json &j, const DetectorParameters &p) {
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

    inline void from_json(const nlohmann::json &j, DetectorParameters &p) {
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


namespace YACCP {
    static nlohmann::json matTo2dArray(const cv::Mat &m) {
        CV_Assert(m.type() == CV_64F);

        nlohmann::json out{nlohmann::json::array()};

        for (auto r{0}; r < m.rows; ++r) {
            nlohmann::json row = nlohmann::json::array();
            for (auto c{0}; c < m.cols; ++c) row.push_back(m.at<double>(r, c));
            out.push_back(row);
        }

        return out;
    }

    static cv::Mat matFrom2dArray(const nlohmann::json &j) {
        if (j.is_null()) return cv::Mat();
        if (!j.is_array() || j.empty()) return cv::Mat();

        const auto rows{static_cast<int>(j.size())};
        const auto cols{static_cast<int>(j.at(0).size())};

        cv::Mat m(rows, cols, CV_64F);

        for (auto r{0}; r < rows; ++r) {
            const auto &row = j.at(r);
            for (auto c{0}; c < cols; ++c) {
                m.at<double>(r, c) = row.at(c).get<double>();
            }
        }

        return m;
    }

    // TODO: check if it's better to keep row column vector formatting.
    static nlohmann::json matTo1dArray(const cv::Mat &m) {
        CV_Assert(m.type() == CV_64F);
        CV_Assert(m.rows == 1 || m.cols == 1);

        const int row = std::max(m.rows, m.cols);

        nlohmann::json out = nlohmann::json::array();

        for (auto i{0}; i < row; ++i) {
            out.push_back(m.rows == 1 ? m.at<double>(0, i) : m.at<double>(i, 0));
        }

        return out;
    }

    static cv::Mat matFrom1dArray(const nlohmann::json &j) {
        if (j.is_null()) return cv::Mat();
        if (!j.is_array()) return cv::Mat();

        const auto rows{static_cast<int>(j.size())};

        cv::Mat m(rows, 1, CV_64F);

        for (auto r{0}; r < rows; ++r) {
            m.at<double>(r, 0) = j.at(r).get<double>();
        }

        return m;
    }

    static nlohmann::json vec3ToArray(const cv::Mat &v) {
        CV_Assert(v.type() == CV_64F);
        CV_Assert((v.rows == 3 && v.cols == 1) || (v.rows == 1 && v.cols == 3));

        return nlohmann::json::array({
            v.rows == 3 ? v.at<double>(0, 0) : v.at<double>(0, 0),
            v.rows == 3 ? v.at<double>(1, 0) : v.at<double>(0, 1),
            v.rows == 3 ? v.at<double>(2, 0) : v.at<double>(0, 2),
        });
    }

    static cv::Mat vec3FromArray(const nlohmann::json &j) {
        if (j.is_null()) return cv::Mat();
        if (!j.is_array() || j.size() != 3) return cv::Mat();

        cv::Mat m(3, 1, CV_64F);
        for (auto r{0}; r < m.rows; ++r) {
            m.at<double>(r, 0) = j.at(r).get<double>();
        }

        return m;
    }

    struct CharucoConfig {
        int openCvDictionaryId;
        cv::Size boardSize;
        float squareLength;
        float markerLength;
        cv::aruco::CharucoParameters charucoParams;
        cv::aruco::DetectorParameters detParams;
    };

    /**
    * @brief All data concerning a camera worker.
    *
    * @param isMaster Indicates if this camera is the master camera.
    * @param isOpen Whenever the camera has been opened successfully.
    * @param isRunning Whenever the camera is running.
    * @param exitCode Exit code of the camera worker thread.
    * @param camName A string representing the camera name for user feedback.
    * @param width The width resolution of the camera.
    * @param height The height resolution of the camera.
    * @param frame The latest frame captured by the camera.
    * @param m Mutex to protect access to the frame.
    * @param frameRequestQ Queue to request a new frame from the slave cameras.
    * @param frameVerifyQ Queue to send frames to the verification thread.
    */
    struct CamData {
        struct ViewData {
            // int windowX;
            int windowX;
            int windowY;
        };

        struct CalibData {
            // Calibration results
            cv::Mat cameraMatrix;
            cv::Mat distCoeffs;
            std::vector<cv::Mat> rvecs;
            std::vector<cv::Mat> tvecs;
            double reprojError;
        };

        struct Info {
            // Camera information
            std::string camName;
            int camId;
            cv::Size resolution;
            bool isMaster;

            ViewData camViewData;
            CalibData calibData;
        };

        struct RuntimeData {
            // Thread information
            bool isOpen{false};
            bool isRunning{false};
            int exitCode;
            cv::Mat frame;
            std::mutex m;

            // Communication
            moodycamel::ReaderWriterQueue<int> frameRequestQ{100};
            moodycamel::BlockingReaderWriterQueue<VerifyTask> frameVerifyQ{100};
        };

        // Camera placement in their respective IDs from left to right, IDs depend on the order of how they are defined.
        // For instance slave cameras [a, b] master camera c then the IDs are [a:0, b:1, c:2].
        Info info;
        RuntimeData runtimeData;
    };

    inline void to_json(nlohmann::json &j, const CharucoConfig &c) {
        j = {
            {
                "boardSize",
                {
                    {"width", c.boardSize.width},
                    {"height", c.boardSize.height}
                }
            },
            {"squareLength", c.squareLength},
            {"markerLength", c.markerLength},
            {"openCvDictionaryId", c.openCvDictionaryId},
            {"charucoParams", c.charucoParams},
            {"detectorParams", c.detParams}
        };
    }

    inline void from_json(const nlohmann::json &j, CharucoConfig &c) {
        j.at("boardSize").at("width").get_to(c.boardSize.width);
        j.at("boardSize").at("height").get_to(c.boardSize.height);
        j.at("squareLength").get_to(c.squareLength);
        j.at("markerLength").get_to(c.markerLength);
        j.at("openCvDictionaryId").get_to(c.openCvDictionaryId);
        j.at("charucoParams").get_to(c.charucoParams);
        j.at("detectorParams").get_to(c.detParams);
    }

    inline void to_json(nlohmann::json &j, const CamData::ViewData &v) {
        j = {
            {"windowX", v.windowX},
            {"windowY", v.windowY}
        };
    }

    inline void from_json(const nlohmann::json &j, CamData::ViewData &v) {
        j.at("windowX").get_to(v.windowX);
        j.at("windowY").get_to(v.windowY);
    }

    inline void to_json(nlohmann::json &j, const CamData::CalibData &c) {
        if (!c.cameraMatrix.empty()) {
            j = {
                {"reprojError", c.reprojError},
                {"cameraMatrix", matTo2dArray(c.cameraMatrix)},
                {"distCoeffs", matTo1dArray(c.distCoeffs)}
            };

            j["rvecs"] = nlohmann::json::array();
            j["tvecs"] = nlohmann::json::array();
            for (size_t i = 0; i < c.rvecs.size(); ++i) {
                j["rvecs"].push_back(vec3ToArray(c.rvecs[i]));
                j["tvecs"].push_back(vec3ToArray(c.tvecs[i]));
            }
        }
    }

    inline void from_json(const nlohmann::json &j, CamData::CalibData &c) {
        j.at("reprojError").get_to(c.reprojError);
        c.cameraMatrix = matFrom2dArray(j.at("cameraMatrix"));
        c.distCoeffs = matFrom1dArray(j.at("distCoeffs"));

        for (const auto &rv : j.at("rvecs")) c.rvecs.emplace_back(vec3FromArray(rv));
        for (const auto &tv : j.at("tvecs")) c.tvecs.emplace_back(vec3FromArray(tv));
    }

    inline void to_json(nlohmann::json &j, const CamData::Info &i) {
        j = {
            {"camName", i.camName},
            {"camId", i.camId},
            {
                "resolution",
                {
                    {"width", i.resolution.width},
                    {"height", i.resolution.height},
                }
            },
            {"isMaster", i.isMaster},
            {"view", i.camViewData},
            {"calibration", i.calibData}
        };
    }

    inline void from_json(const nlohmann::json &j, CamData::Info &i) {
        j.at("camName").get_to(i.camName);
        j.at("camId").get_to(i.camId);
        j.at("resolution").at("width").get_to(i.resolution.width);
        j.at("resolution").at("height").get_to(i.resolution.height);
        j.at("isMaster").get_to(i.isMaster);
        j.at("view").get_to(i.camViewData);
        if (j.contains("calibration") && !j.at("calibration").is_null()) j.at("calibration").get_to(i.calibData);
    }

    inline void to_json(nlohmann::json &j, const CamData &c) {
        j = c.info;
        // runtimeData intentionally excluded
    }

    inline void from_json(const nlohmann::json &j, CamData &c) {
        j.at("info").get_to(c.info);
    }
}

#endif //STEREO_CAMERA_CALIBRATION_CAM_DATA_HPP
