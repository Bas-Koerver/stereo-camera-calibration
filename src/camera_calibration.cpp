#include "camera_calibration.hpp"

#include <CLI/Validators.hpp>

#include "utility.hpp"

namespace YACCP {
    CameraCalibration::CameraCalibration(
        const cv::aruco::CharucoDetector& charucoDetector,
        const std::filesystem::path& dataPath,
        float cornerMin)
        : charucoDetector_(charucoDetector),
          dataPath_(dataPath),
          cornerMin_(cornerMin) {
    }

    void CameraCalibration::monoCalibrate(const std::string& jobName) {
        // Check if given job ID exists.
        jobPath_ = dataPath_ / jobName;

        // Check if the job has verified images
        if (!Utility::isNonEmptyDirectory(jobPath_ / "images" / "verified")) {
            std::cerr << "No verified images found for job: " << jobName << "\n";
            return;
        }

        // Check if job_data.json exists
        if (!exists(jobPath_ / "job_data.json")) {
            throw std::runtime_error("No camera data was found.\nStopping!");
        }

        std::vector<std::filesystem::path> cams;
        std::vector<std::filesystem::path> files;
        Config::FileConfig fileConfig;

        // Load job_data.json and save it to the FileConfig struct and CamData struct
        nlohmann::json j = Utility::loadJobDataFromFile(jobPath_);
        j.at("config").get_to(fileConfig);

        std::vector<CamData> camDatas(fileConfig.recordingConfig.workers.size());
        for (auto& [key, obj] : j.at("cams").items()) {
            camDatas[obj.at("camId").get<int>()].info = obj.get<CamData::Info>();
        }

        for (auto& [info, runtimeData] : camDatas) {
            std::string camDir{"cam_" + std::to_string(info.camIndexId)};
            std::filesystem::path camPath{jobPath_ / "images" / "verified" / camDir};

            if (!exists(camPath))
                throw std::runtime_error("Directory for " + camDir + " does not exist.");

            if (!Utility::isNonEmptyDirectory(camPath))
                throw std::runtime_error("Camera directory " + camDir + " does not contain any data");

            cams.emplace_back(camPath);
        }

        cv::aruco::CharucoBoard board{charucoDetector_.getBoard()};
        cv::Size boardSize = board.getChessboardSize();
        int cornerAmount{(boardSize.width - 1) * (boardSize.height - 1)};

        std::ranges::sort(cams);

        // Get all the file indexes, only a single camera directory needs to be looked at,
        // since the filenames are the same across the multiple cameras.
        for (auto const& entry : std::filesystem::directory_iterator(cams.front())) {
            if (!entry.is_regular_file()) continue;
            files.push_back(entry.path().filename());
        }

        auto i{0};
        for (auto& cam : cams) {
            std::vector<std::vector<cv::Point3f>> allObjPoints;
            std::vector<std::vector<cv::Point2f>> allImgPoints;


            for (auto& file : files) {
                std::vector<cv::Point3f> objPoints;
                std::vector<cv::Point2f> imgPoints;
                // TODO: add progressbar for image loading & detection.
                cv::Mat img{cv::imread((cam / file).string(), cv::IMREAD_GRAYSCALE)};

                auto results{
                    Utility::findBoard(charucoDetector_, img, std::floor(static_cast<float>(cornerAmount) * cornerMin_))
                };

                if (!results.boardFound) continue;

                board.matchImagePoints(results.charucoCorners,
                                       results.charucoIds,
                                       objPoints,
                                       imgPoints);
                allObjPoints.push_back(objPoints);
                allImgPoints.push_back(imgPoints);
            }

            camDatas[i].info.calibData.reprojError = cv::calibrateCamera(allObjPoints,
                                                                         allImgPoints,
                                                                         cv::Size(camDatas[i].info.resolution.width,
                                                                             camDatas[i].info.resolution.height),
                                                                         camDatas[i].info.calibData.cameraMatrix,
                                                                         camDatas[i].info.calibData.distCoeffs,
                                                                         camDatas[i].info.calibData.rvecs,
                                                                         camDatas[i].info.calibData.tvecs);

            std::cout << "Calibration of cam: " << camDatas[i].info.camName << "\n  ID: " << camDatas[i].info.camIndexId
                << ", done \n";

            i++;
        }
        // Save JSON to file.
        Utility::saveJobDataToFile(jobPath_, fileConfig, camDatas);
    }

    void CameraCalibration::stereoCalibrate(const std::string& jobName) {
    }
}

namespace YACCP::Calibration {
    void monoCalibrate(const cv::aruco::CharucoDetector& charucoDetector,
                                    std::vector<CamData>& camDatas,
                                    const Config::FileConfig& fileConfig,
                                    const std::filesystem::path& jobPath) {
        std::vector<std::filesystem::path> cams;
        std::vector<std::filesystem::path> files;

        for (auto& [info, runtimeData] : camDatas) {
            std::string camDir{"cam_" + std::to_string(info.camIndexId)};
            std::filesystem::path camPath{jobPath / "images" / "verified" / camDir};

            if (!exists(camPath))
                throw std::runtime_error("Directory for " + camDir + " does not exist.");

            if (!Utility::isNonEmptyDirectory(camPath))
                throw std::runtime_error("Camera directory " + camDir + " does not contain any data");

            cams.emplace_back(camPath);
        }

        cv::aruco::CharucoBoard board{charucoDetector.getBoard()};
        cv::Size boardSize = board.getChessboardSize();
        int cornerAmount{(boardSize.width - 1) * (boardSize.height - 1)};

        std::ranges::sort(cams);

        // Get all the file indexes, only a single camera directory needs to be looked at,
        // since the filenames are the same across the multiple cameras.
        for (auto const& entry : std::filesystem::directory_iterator(cams.front())) {
            if (!entry.is_regular_file()) continue;
            files.push_back(entry.path().filename());
        }

        auto i{0};
        for (auto& cam : cams) {
            std::vector<std::vector<cv::Point3f>> allObjPoints;
            std::vector<std::vector<cv::Point2f>> allImgPoints;


            for (auto& file : files) {
                std::vector<cv::Point3f> objPoints;
                std::vector<cv::Point2f> imgPoints;
                // TODO: add progressbar for image loading & detection.
                cv::Mat img{cv::imread((cam / file).string(), cv::IMREAD_GRAYSCALE)};

                Utility::CharucoResults results{
                    Utility::findBoard(charucoDetector,
                                       img,
                                       std::floor(
                                           static_cast<float>(cornerAmount) * fileConfig.detectionConfig.cornerMin))
                };

                if (!results.boardFound) continue;

                board.matchImagePoints(results.charucoCorners,
                                       results.charucoIds,
                                       objPoints,
                                       imgPoints);
                allObjPoints.push_back(objPoints);
                allImgPoints.push_back(imgPoints);
            }

            camDatas[i].info.calibData.reprojError = cv::calibrateCamera(allObjPoints,
                                                                         allImgPoints,
                                                                         cv::Size(camDatas[i].info.resolution.width,
                                                                             camDatas[i].info.resolution.height),
                                                                         camDatas[i].info.calibData.cameraMatrix,
                                                                         camDatas[i].info.calibData.distCoeffs,
                                                                         camDatas[i].info.calibData.rvecs,
                                                                         camDatas[i].info.calibData.tvecs);

            std::cout << "Calibration of cam: " << camDatas[i].info.camName << "\n  ID: " << camDatas[i].info.camIndexId
                << ", done \n";

            i++;
        }
    }
}
