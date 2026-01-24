#include "create_board.hpp"

#include "../global_variables/config_defaults.hpp"
#include "../global_variables/program_defaults.hpp"

#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>

namespace {
    void generateVideo(const cv::Mat& image, const cv::Size size, const std::filesystem::path& jobPath) {
        const std::string filename{(jobPath / YACCP::GlobalVariables::boardVideoFileName).string()};

        // H264 codec seems to work on most devices
        const int codec{cv::VideoWriter::fourcc('h', '2', '6', '4')};

        cv::Mat imageBgr;
        cv::cvtColor(image, imageBgr, cv::COLOR_GRAY2BGR);

        const cv::Mat blankImage{cv::Mat::zeros(size, CV_8UC3)};
        cv::VideoWriter writer(filename, codec, static_cast<double>(YACCP::GlobalVariables::videoFps), size, true);

        if (!writer.isOpened()) {
            throw std::runtime_error{"Could not open the output video for writing: " + filename + "\n"};
        }

        // Ten-second video.
        for (auto i{0}; i < YACCP::GlobalVariables::videoFps * 5; i++) {
            writer.write(imageBgr);
            writer.write(blankImage);
        }
        writer.release();
    }
}

namespace YACCP::CreateBoard {
    void listJobs(const std::filesystem::path& dataPath) {
        std::cout << "Jobs missing a board image and/or video: \n";
        for (auto const& entry : std::filesystem::directory_iterator(dataPath)) {
            if (!entry.is_directory()) {
                continue;
            }

            const bool jobDataFound{std::filesystem::exists(entry.path() / GlobalVariables::jobDataFileName)};
            const bool imageFound{std::filesystem::exists(entry.path() / GlobalVariables::boardImageFileName)};
            const bool videoFound{std::filesystem::exists(entry.path() / GlobalVariables::boardVideoFileName)};

            if (!jobDataFound || (imageFound && videoFound)) {
                continue;
            }

            std::cout << "  " << entry.path().filename() << "\n";
        }
    }


    // TODO: add different calibration patterns: https://github.com/opencv/opencv/blob/74addff3d065e53ce2ea16e2861d7711fb549780/samples/cpp/tutorial_code/calib3d/camera_calibration/camera_calibration.cpp#L156
    void charuco(const Config::FileConfig& fileConfig,
                 const CLI::BoardCreationCmdConfig& boardCreationConfig,
                 const std::filesystem::path& jobPath) {
        const cv::aruco::Dictionary dictionary{
            cv::aruco::getPredefinedDictionary(fileConfig.detectionConfig.openCvDictionaryId)
        };
        const cv::aruco::CharucoBoard board{
            fileConfig.boardConfig.boardSize,
            static_cast<float>(fileConfig.boardConfig.squarePixelLength),
            static_cast<float>(fileConfig.boardConfig.markerPixelLength),
            dictionary
        };

        const int width{
            (fileConfig.boardConfig.boardSize.width * fileConfig.boardConfig.squarePixelLength) + (2 * fileConfig.
                boardConfig.marginSize)
        };
        const int height{
            (fileConfig.boardConfig.boardSize.height * fileConfig.boardConfig.squarePixelLength) + (2 * fileConfig.
                boardConfig.marginSize)
        };
        cv::Mat boardImage;
        const cv::Size imageSize(width, height);

        board.generateImage(imageSize,
                            boardImage,
                            fileConfig.boardConfig.marginSize,
                            fileConfig.boardConfig.borderBits);

        if (boardCreationConfig.generateImage) {
            (void)cv::imwrite((jobPath / "board.png").string(), boardImage);
        }

        if (boardCreationConfig.generateVideo) {
            generateVideo(boardImage, imageSize, jobPath);
        }
    }
} // namespace YACCP
