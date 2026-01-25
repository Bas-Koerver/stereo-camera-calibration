#include "calibration_runner.hpp"

#include "../camera_calibration.hpp"
#include "../utility.hpp"


namespace YACCP::Executor {
    void runCalibration(CLI::CliCmdConfig& cliCmdConfig,
                        CLI::CliCmds& cliCmds,
                        std::filesystem::path path,
                        const std::string& dateTime) {
        const std::filesystem::path dataPath{path / "data"};
        std::filesystem::path jobPath = dataPath / cliCmdConfig.calibrationCmdConfig.jobId;

        Utility::checkJobPath(dataPath, cliCmdConfig.calibrationCmdConfig.jobId);

        // Check if the job has verified images
        if (!Utility::isNonEmptyDirectory(jobPath / "images" / "verified")) {
            throw std::runtime_error("No verified images found for job: " + cliCmdConfig.calibrationCmdConfig.jobId);
        }

        // Load config from JSON file
        Config::FileConfig fileConfig;

        nlohmann::json j = Utility::loadJobDataFromFile(jobPath);
        j.at("config").get_to(fileConfig);

        std::vector<CamData> camDatas(fileConfig.recordingConfig.workers.size());
        for (auto& [key, obj] : j.at("cams").items()) {
            camDatas[obj.at("camId").get<int>()].info = obj.get<CamData::Info>();
        }

        std::vector<StereoCalibData> stereoCalibDatas;
        if (j.contains("stereoCalib")) {
            for (auto& [key, obj] : j.at("stereoCalib").items()) {
                auto stereoCalibData = obj.get<StereoCalibData>();
                stereoCalibDatas.emplace_back(stereoCalibData);
            }
        }

        // Variable setup based on config.
        cv::aruco::Dictionary dictionary{
            cv::aruco::getPredefinedDictionary(fileConfig.detectionConfig.openCvDictionaryId)
        };
        cv::aruco::CharucoParameters charucoParams;
        cv::aruco::DetectorParameters detParams;
        cv::aruco::CharucoBoard board{
            fileConfig.boardConfig.boardSize,
            fileConfig.boardConfig.squareLength,
            fileConfig.boardConfig.markerLength,
            dictionary
        };
        cv::aruco::CharucoDetector charucoDetector(board, charucoParams, detParams);

        if (*cliCmds.calibrationCmds.mono || true) {
            // cameraCalibration.monoCalibrate(cliCmdConfig.calibrationCmdConfig.jobId);
            Calibration::monoCalibrate(charucoDetector, camDatas, fileConfig, jobPath);
        } else if (*cliCmds.calibrationCmds.stereo) {
            Calibration::pairWiseStereoCalibrate(charucoDetector, camDatas, stereoCalibDatas, fileConfig, jobPath);
        } else {
            std::cout << "base calibration called\n";
        }

        // Save JSON to file.
        Utility::saveJobDataToFile(jobPath, fileConfig, &camDatas, &stereoCalibDatas);
    }
} // YACCP::Executor