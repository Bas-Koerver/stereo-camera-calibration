#ifndef YACCP_SRC_RECORDING_JOB_DATA_HPP
#define YACCP_SRC_RECORDING_JOB_DATA_HPP
#include "detection_validator.hpp"

namespace YACCP {
    static nlohmann::json matTo2dArray(const cv::Mat& m) {
        CV_Assert(m.type() == CV_64F);

        nlohmann::json out{nlohmann::json::array()};

        for (auto r{0}; r < m.rows; ++r) {
            nlohmann::json row = nlohmann::json::array();
            for (auto c{0}; c < m.cols; ++c) row.push_back(m.at<double>(r, c));
            out.push_back(row);
        }

        return out;
    }


    static cv::Mat matFrom2dArray(const nlohmann::json& j) {
        if (j.is_null()) return cv::Mat();
        if (!j.is_array() || j.empty()) return cv::Mat();

        const auto rows{static_cast<int>(j.size())};
        const auto cols{static_cast<int>(j.at(0).size())};

        cv::Mat m(rows, cols, CV_64F);

        for (auto r{0}; r < rows; ++r) {
            const auto& row = j.at(r);
            for (auto c{0}; c < cols; ++c) {
                m.at<double>(r, c) = row.at(c).get<double>();
            }
        }

        return m;
    }


    // TODO: check if it's better to keep row column vector formatting.
    static nlohmann::json matTo1dArray(const cv::Mat& m) {
        CV_Assert(m.type() == CV_64F);
        CV_Assert(m.rows == 1 || m.cols == 1);

        const int row = std::max(m.rows, m.cols);

        nlohmann::json out = nlohmann::json::array();

        for (auto i{0}; i < row; ++i) {
            out.push_back(m.rows == 1 ? m.at<double>(0, i) : m.at<double>(i, 0));
        }

        return out;
    }


    static cv::Mat matFrom1dArray(const nlohmann::json& j) {
        if (j.is_null()) return cv::Mat();
        if (!j.is_array()) return cv::Mat();

        const auto rows{static_cast<int>(j.size())};

        cv::Mat m(rows, 1, CV_64F);

        for (auto r{0}; r < rows; ++r) {
            m.at<double>(r, 0) = j.at(r).get<double>();
        }

        return m;
    }


    static nlohmann::json vec3ToArray(const cv::Mat& v) {
        CV_Assert(v.type() == CV_64F);
        CV_Assert((v.rows == 3 && v.cols == 1) || (v.rows == 1 && v.cols == 3));

        return nlohmann::json::array({
            v.rows == 3 ? v.at<double>(0, 0) : v.at<double>(0, 0),
            v.rows == 3 ? v.at<double>(1, 0) : v.at<double>(0, 1),
            v.rows == 3 ? v.at<double>(2, 0) : v.at<double>(0, 2),
        });
    }


    static cv::Mat vec3FromArray(const nlohmann::json& j) {
        if (j.is_null()) return cv::Mat();
        if (!j.is_array() || j.size() != 3) return cv::Mat();

        cv::Mat m(3, 1, CV_64F);
        for (auto r{0}; r < m.rows; ++r) {
            m.at<double>(r, 0) = j.at(r).get<double>();
        }

        return m;
    }


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
        // TODO: Remove this in favor of current config setup (horizontal views.)
        struct ViewData {
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
            int camIndexId;
            cv::Size resolution;
            bool isMaster;

            ViewData ViewData;
            CalibData calibData;
        };

        struct RuntimeData {
            // Thread information
            std::atomic<bool> isOpen{false};
            std::atomic<bool> isRunning{false};
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

    struct StereoCalibData {
        int camLeftId;
        int camRightId;
        cv::Mat rotationMatrix;
        cv::Mat translationMatrix;
        cv::Mat essentialMatrix;
        cv::Mat fundamentalMatrix;
    };


    inline void to_json(nlohmann::json& j, const CamData::ViewData& v) {
        j = {
            {"windowX", v.windowX},
            {"windowY", v.windowY}
        };
    }


    inline void from_json(const nlohmann::json& j, CamData::ViewData& v) {
        j.at("windowX").get_to(v.windowX);
        j.at("windowY").get_to(v.windowY);
    }


    inline void to_json(nlohmann::json& j, const CamData::CalibData& c) {
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


    inline void from_json(const nlohmann::json& j, CamData::CalibData& c) {
        j.at("reprojError").get_to(c.reprojError);
        c.cameraMatrix = matFrom2dArray(j.at("cameraMatrix"));
        c.distCoeffs = matFrom1dArray(j.at("distCoeffs"));

        for (const auto& rv : j.at("rvecs")) c.rvecs.emplace_back(vec3FromArray(rv));
        for (const auto& tv : j.at("tvecs")) c.tvecs.emplace_back(vec3FromArray(tv));
    }


    inline void to_json(nlohmann::json& j, const CamData::Info& i) {
        j = {
            {"camName", i.camName},
            {"camId", i.camIndexId},
            {
                "resolution",
                {
                    {"width", i.resolution.width},
                    {"height", i.resolution.height},
                }
            },
            {"isMaster", i.isMaster},
            {"view", i.ViewData},
            {"calibration", i.calibData}
        };
    }


    inline void from_json(const nlohmann::json& j, CamData::Info& i) {
        j.at("camName").get_to(i.camName);
        j.at("camId").get_to(i.camIndexId);
        j.at("resolution").at("width").get_to(i.resolution.width);
        j.at("resolution").at("height").get_to(i.resolution.height);
        j.at("isMaster").get_to(i.isMaster);
        j.at("view").get_to(i.ViewData);
        if (j.contains("calibration") && !j.at("calibration").is_null()) j.at("calibration").get_to(i.calibData);
    }


    inline void to_json(nlohmann::json& j, const CamData& c) {
        j = c.info;
        // runtimeData intentionally excluded
    }


    inline void from_json(const nlohmann::json& j, CamData& c) {
        j.at("info").get_to(c.info);
    }


    inline void to_json(nlohmann::json& j, const StereoCalibData& s) {
        j = {
            {"camLeftId", s.camLeftId},
            {"camRightId", s.camRightId},
            {"rotationMatrix", matTo2dArray(s.rotationMatrix)},
            {"translationMatrix", matTo1dArray(s.translationMatrix)},
            {"essentialMatrix", matTo2dArray(s.essentialMatrix)},
            {"fundamentalMatrix", matTo2dArray(s.fundamentalMatrix)}
        };
    }


    inline void from_json(const nlohmann::json& j, StereoCalibData& s) {
        s.camLeftId = j.at("camLeftId");
        s.camRightId = j.at("camRightId");
        s.rotationMatrix = matFrom2dArray(j.at("rotationMatrix"));
        s.translationMatrix = matFrom1dArray(j.at("translationMatrix"));
        s.essentialMatrix = matFrom2dArray(j.at("essentialMatrix"));
        s.fundamentalMatrix = matFrom2dArray(j.at("fundamentalMatrix"));
    }
}

#endif //YACCP_SRC_RECORDING_JOB_DATA_HPP