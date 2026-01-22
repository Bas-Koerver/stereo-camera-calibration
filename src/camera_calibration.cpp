#include "camera_calibration.hpp"

#include <CLI/Validators.hpp>

#include "utility.hpp"

namespace YACCP::Calibration {
    static void filterByOverlapIds(
        const Utility::CharucoResults& resultsLeft,
        const Utility::CharucoResults& resultsRight,
        std::vector<cv::Point2f>& overlapCornersLeft,
        std::vector<cv::Point2f>& overlapCornersRight,
        std::vector<int>& outIds) {
        // index lookup
        std::unordered_map<int, std::size_t> leftIds;
        leftIds.reserve(resultsLeft.charucoIds.size());
        for (std::size_t i{0}; i < resultsLeft.charucoIds.size(); ++i)
            leftIds[resultsLeft.charucoIds[i]] = i;

        std::unordered_map<int, std::size_t> rightIds;
        rightIds.reserve(resultsRight.charucoIds.size());
        for (std::size_t i{0}; i < resultsRight.charucoIds.size(); ++i)
            rightIds[resultsRight.charucoIds[i]] = i;

        // overlap ids (optionally sort for deterministic ordering)
        const std::vector overlapIds{Utility::intersection(resultsLeft.charucoIds, resultsRight.charucoIds)};

        overlapCornersLeft.reserve(overlapIds.size());
        overlapCornersRight.reserve(overlapIds.size());
        outIds.reserve(overlapIds.size());

        for (int id : overlapIds) {
            auto iteratorLeft{leftIds.find(id)};
            auto iteratorRight{rightIds.find(id)};
            if (iteratorLeft == leftIds.end() || iteratorRight == rightIds.end())
                continue;

            outIds.push_back(id);
            overlapCornersLeft.push_back(resultsLeft.charucoCorners[iteratorLeft->second]);
            overlapCornersRight.push_back(resultsRight.charucoCorners[iteratorRight->second]);
        }
    }

    void getCamDirs(std::vector<std::filesystem::path>& cams,
                    std::vector<CamData>& camDatas,
                    const std::filesystem::path& jobPath) {
        for (auto& [info, runtimeData] : camDatas) {
            std::string camDir{"cam_" + std::to_string(info.camIndexId)};
            std::filesystem::path camPath{jobPath / "images" / "verified" / camDir};

            if (!exists(camPath))
                throw std::runtime_error("Directory for " + camDir + " does not exist.");

            if (!Utility::isNonEmptyDirectory(camPath))
                throw std::runtime_error("Camera directory " + camDir + " does not contain any data");

            cams.emplace_back(camPath);
        }

        std::ranges::sort(cams);
    }

    void getImages(const std::filesystem::path& cam, std::vector<std::filesystem::path>& files) {
        for (auto const& entry : std::filesystem::directory_iterator(cam)) {
            if (!entry.is_regular_file()) continue;
            files.push_back(entry.path().filename());
        }
    }

    void monoCalibrate(const cv::aruco::CharucoDetector& charucoDetector,
                       std::vector<CamData>& camDatas,
                       const Config::FileConfig& fileConfig,
                       const std::filesystem::path& jobPath) {
        std::vector<std::filesystem::path> cams;
        std::vector<std::filesystem::path> files;

        // Get all camera directories in the given job path.
        getCamDirs(cams, camDatas, jobPath);
        // Get all the file indexes, only a single camera directory needs to be looked at,
        // since the filenames are the same across the multiple cameras.
        getImages(cams.front(), files);

        cv::aruco::CharucoBoard board{charucoDetector.getBoard()};
        cv::Size boardSize = board.getChessboardSize();
        int cornerAmount{(boardSize.width - 1) * (boardSize.height - 1)};

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

    void pairWiseStereoCalibrate(const cv::aruco::CharucoDetector& charucoDetector,
                                 std::vector<CamData>& camDatas,
                                 std::vector<StereoCalibData>& stereoCalibDatas,
                                 const Config::FileConfig& fileConfig,
                                 const std::filesystem::path& jobPath) {
        std::vector<std::filesystem::path> cams;
        std::vector<std::filesystem::path> files;

        // Get all camera directories in the given job path.
        getCamDirs(cams, camDatas, jobPath);
        // Get all the file indexes, only a single camera directory needs to be looked at,
        // since the filenames are the same across the multiple cameras.
        getImages(cams.front(), files);

        cv::aruco::CharucoBoard board{charucoDetector.getBoard()};
        cv::Size boardSize = board.getChessboardSize();
        int cornerAmount{(boardSize.width - 1) * (boardSize.height - 1)};

        for (auto& [info, runtimeData] : camDatas) {
            if (info.calibData.cameraMatrix.empty() || info.calibData.distCoeffs.empty())
                throw std::runtime_error("Camera: " + info.camName + " with ID: " + std::to_string(info.camIndexId) +
                    "\nIs missing its camera matrix or distance coefficients vector, did you run mono calibration?");
        }

        for (auto left{0}; left < cams.size(); ++left) {
            for (auto right{left + 1}; right < cams.size(); ++right) {
                std::vector<std::vector<cv::Point3f>> allObjPoints;
                std::vector<std::vector<cv::Point2f>> allImgPointsLeft, allImgPointsRight;
                StereoCalibData stereoCalibData;
                stereoCalibData.camLeftId = left;
                stereoCalibData.camRightId = right;

                for (auto& file : files) {
                    // TODO: add progressbar for image loading & detection.
                    std::vector<cv::Point3f> objPointsLeft, objPointsRight;
                    std::vector<cv::Point2f> imgPointsLeft, imgPointsRight;
                    std::vector<cv::Point2f> overlapCornersLeft, overlapCornersRight;
                    std::vector<int> overlapIds;

                    cv::Mat imgLeft{cv::imread((cams[left] / file).string(), cv::IMREAD_GRAYSCALE)};
                    cv::Mat imgRight{cv::imread((cams[right] / file).string(), cv::IMREAD_GRAYSCALE)};

                    Utility::CharucoResults resultsLeft{
                        Utility::findBoard(
                            charucoDetector,
                            imgLeft,
                            std::floor(static_cast<float>(cornerAmount) * fileConfig.detectionConfig.cornerMin))
                    };
                    Utility::CharucoResults resultsRight{
                        Utility::findBoard(
                            charucoDetector,
                            imgRight,
                            std::floor(static_cast<float>(cornerAmount) * fileConfig.detectionConfig.cornerMin))
                    };

                    if (!resultsLeft.boardFound || !resultsRight.boardFound) continue;

                    filterByOverlapIds(resultsLeft, resultsRight, overlapCornersLeft, overlapCornersRight, overlapIds);

                    board.matchImagePoints(overlapCornersLeft, overlapIds, objPointsLeft, imgPointsLeft);
                    board.matchImagePoints(overlapCornersRight, overlapIds, objPointsRight, imgPointsRight);

                    allObjPoints.emplace_back(objPointsLeft);
                    allImgPointsLeft.emplace_back(imgPointsLeft);
                    allImgPointsRight.emplace_back(imgPointsRight);
                }

                std::cout << "Starting stereo calibration for cameras " + std::to_string(left) + " and " +
                    std::to_string(right) + "\n";

                auto calibFlags{cv::CALIB_FIX_INTRINSIC};
                cv::stereoCalibrate(allObjPoints,
                                    allImgPointsLeft,
                                    allImgPointsRight,
                                    camDatas[left].info.calibData.cameraMatrix,
                                    camDatas[left].info.calibData.distCoeffs,
                                    camDatas[right].info.calibData.cameraMatrix,
                                    camDatas[right].info.calibData.distCoeffs,
                                    cv::Size(0, 0),
                                    stereoCalibData.rotationMatrix,
                                    stereoCalibData.translationMatrix,
                                    stereoCalibData.essentialMatrix,
                                    stereoCalibData.fundamentalMatrix,
                                    cv::noArray(),
                                    calibFlags);
                stereoCalibDatas.emplace_back(stereoCalibData);
            }
        }
    }
}
