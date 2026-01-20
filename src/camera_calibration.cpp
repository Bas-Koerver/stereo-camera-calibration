#include "camera_calibration.hpp"

#include "utility.hpp"
#include "recoding/job_data.hpp"

#include <fstream>
// #include "tools/image_validator.hpp"


namespace YACCP {
    CameraCalibration::CameraCalibration(
        const cv::aruco::CharucoDetector& charucoDetector,
        const std::filesystem::path& dataPath,
        float cornerMin)
        : charucoDetector_(charucoDetector),
          dataPath_(dataPath),
          cornerMin_(cornerMin) {
    }

    // void Calibration::processImage(std::vector<cv::Point3f> objPoints, std::vector<cv::Point2f> imagePoints) {
    //
    // }

    void CameraCalibration::monoCalibrate(const std::string& jobName) {
        jobPath_ = dataPath_ / jobName;
        if (!exists(jobPath_)) {
            std::cerr << "Job: " << jobName << " does not exist in the given path: " << dataPath_ << "\n";
            return;
        }

        if (!Utility::isNonEmptyDirectory(jobPath_ / "images" / "verified")) {
            std::cerr << "No verified images found for job: " << jobName << "\n";
            return;
        }

        std::vector<std::filesystem::path> cams;
        std::vector<std::filesystem::path> files;
        std::vector<std::vector<cv::Point3f>> allObjPoints;
        std::vector<std::vector<cv::Point2f>> allImgPoints;

        cv::aruco::CharucoBoard board{charucoDetector_.getBoard()};
        cv::Size boardSize = board.getChessboardSize();
        int cornerAmount{(boardSize.width - 1) * (boardSize.height - 1)};

        // Get all the camera directories
        for (auto const& entry : std::filesystem::directory_iterator(jobPath_ / "images" / "verified")) {
            if (!entry.is_directory()) continue;
            cams.push_back(entry.path().filename());
        }
        // Get all the file indexes, only a single camera directory needs to be looked at,
        // since the filenames are the same across the multiple cameras.
        for (auto const& entry : std::filesystem::directory_iterator(jobPath_ / "images" / "verified" / cams[0])) {
            if (!entry.is_regular_file()) continue;
            files.push_back(entry.path().filename());
        }

        if (!exists(jobPath_ / "job_data.json")) {
            std::cerr << "No camera data was found.\nStopping!\n";
            return;
        }


        std::ifstream f{jobPath_ / "job_data.json"};
        if (!f) {
            std::cerr << "Camera data exists but cannot be opened.\nStopping!\n";
            return;
        }

        nlohmann::json jsonObj{nlohmann::json::parse(f)};
        std::vector<YACCP::CamData::Info> camDatas(cams.size());
        try {
            for (auto& [key, j] : jsonObj.at("cams").items()) {
                YACCP::CamData cam;
                camDatas[j.at("camId").get<int>()] = j.get<YACCP::CamData::Info>();
            }
        } catch (std::exception& e) {
            std::cerr << e.what() << '\n';\
            return;
        }

        auto i{0};
        for (auto& cam : cams) {
            cv::Mat cameraMatrix(3, 3, CV_64F);
            cv::Mat distCoeffs;
            std::vector<cv::Mat> rvecs;
            std::vector<cv::Mat> tvecs;


            for (auto& file : files) {
                std::vector<cv::Point3f> objPoints;
                std::vector<cv::Point2f> imgPoints;

                cv::Mat img{cv::imread((jobPath_ / "images/verified" / cam / file).string(), cv::IMREAD_GRAYSCALE)};

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

            camDatas[i].calibData.reprojError = cv::calibrateCamera(allObjPoints,
                                                                    allImgPoints,
                                                                    cv::Size(camDatas[i].resolution.width,
                                                                             camDatas[i].resolution.height),
                                                                    camDatas[i].calibData.cameraMatrix,
                                                                    camDatas[i].calibData.distCoeffs,
                                                                    camDatas[i].calibData.rvecs,
                                                                    camDatas[i].calibData.tvecs);

            std::cout << "Calibration of cam: " << camDatas[i].camName << "ID: " << camDatas[i].camIndexId << "done \n";


            i++;
        }
        // Save json to file.
        std::ofstream outfile(jobPath_ / "job_data.json");
        for (auto i{0}; i < camDatas.size(); ++i) {
            jsonObj["cams"]["cam_" + std::to_string(i)] = camDatas[i];
        }
        outfile << jsonObj.dump(4);
    }
}