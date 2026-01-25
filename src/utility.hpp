#ifndef YACCP_SRC_UTILITY_HPP
#define YACCP_SRC_UTILITY_HPP
#include "config/orchestrator.hpp"

#include "recoding/job_data.hpp"

#include <nlohmann/json.hpp>

#include <opencv2/objdetect/charuco_detector.hpp>

namespace YACCP::Utility {
    struct AlternativeBuffer {
        // "\x1b" Escape character
        // "[?1049h" enables the alternative buffer
        // "[?1049h" disables the alternative buffer
        // "[2J" Clears entire screen
        // "[1;1H" Moves cursor to the top-left corner

        void enable() {
            if (altBufferEnabled_) {
                return;
            }
            altBufferEnabled_ = true;
            std::cout << "\x1b[?1049h\x1b[2J\x1b[1;1H" << std::flush;
        }


        void disable() {
            if (!altBufferEnabled_) {
                return;
            }
            altBufferEnabled_ = false;
            std::cout << "\x1b[?1049l" << std::flush;
        }


        ~AlternativeBuffer() {
            disable();
        }


    private:
        bool altBufferEnabled_{false};
    };

    struct CharucoResults {
        bool boardFound = false;
        std::vector<int> markerIds;
        std::vector<std::vector<cv::Point2f> > markerCorners;
        std::vector<int> charucoIds;
        std::vector<cv::Point2f> charucoCorners;
    };

    // template <typename T>
    // [[nodiscard]] static std::vector<T> intersection(std::vector<T> vec1, std::vector<T> vec2);

    template <typename T>
    std::vector<T> intersection(std::vector<T> vec1, std::vector<T> vec2) {
        std::vector<T> vecOutput;

        std::sort(vec1.begin(), vec1.end());
        std::sort(vec2.begin(), vec2.end());

        std::set_intersection(vec1.begin(),
                              vec1.end(),
                              vec2.begin(),
                              vec2.end(),
                              std::back_inserter(vecOutput));

        return vecOutput;
    }


    void clearScreen();

    [[nodiscard]] CharucoResults findBoard(const cv::aruco::CharucoDetector& charucoDetector,
                                           const cv::Mat& gray,
                                           int cornerMin);

    [[nodiscard]] std::string getCurrentDateTime();

    [[nodiscard]] bool isNonEmptyDirectory(const std::filesystem::path& path);

    [[nodiscard]] std::ifstream openFile(const std::filesystem::path& path, const std::string& fileName);

    void checkDataPath(const std::filesystem::path& dataPath);

    void checkJobPath(const std::filesystem::path& dataPath, const std::string& jobId);

    void checkJobDataAvailable(const std::filesystem::path& jobPath);

    [[nodiscard]] nlohmann::json loadJobDataFromFile(const std::filesystem::path& path);

    void saveJobDataToFile(const std::filesystem::path& jobPath,
                           Config::FileConfig& fileConfig,
                           const std::vector<CamData>* camDatas = nullptr,
                           const std::vector<StereoCalibData>* stereoCalibDatas = nullptr);

    [[nodiscard]] nlohmann::json parseJsonFromFile(std::ifstream & file);

    [[nodiscard]] bool askYesNo();
} // YACCP::Utility


#endif //YACCP_SRC_UTILITY_HPP