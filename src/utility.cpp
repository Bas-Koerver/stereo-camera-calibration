#include "utility.hpp"

#include "global_variables/program_defaults.hpp"

#include <filesystem>
#include <fstream>

#include <opencv2/core/mat.hpp>
#include <opencv2/objdetect/aruco_detector.hpp>


namespace YACCP::Utility {
    void clearScreen() {
        // "\x1b" Escape character
        // "[2J" Clears entire screen
        // "[1;1H" Moves cursor to the top-left corner
        // https://medium.com/@ryan_forrester_/c-screen-clearing-how-to-guide-cff5bf764ccd
        std::cout << "\x1b[2J\x1b[1;1H" << std::flush;
    }

    // MISRA deviation: OpenCV Charuco API requires std::vector
    CharucoResults findBoard(const cv::aruco::CharucoDetector& charucoDetector,
                             const cv::Mat& gray,
                             const int cornerMin) {
        CharucoResults charucoResults;

        charucoDetector.detectBoard(gray,
                                    charucoResults.charucoCorners,
                                    charucoResults.charucoIds,
                                    charucoResults.markerCorners,
                                    charucoResults.markerIds);

        if (charucoResults.charucoCorners.size() > cornerMin) {
            charucoResults.boardFound = true;
        }
        return charucoResults;
    }

    std::_Timeobj<char, const tm*> getCurrentDateTime() {
        // Get the current date and time.
        const auto now = std::chrono::system_clock::now();
        const auto localTime = std::chrono::system_clock::to_time_t(now);

        // Generate timestamp at UTC +0
        return std::put_time(std::gmtime(&localTime), "%F_%H-%M-%S");
    }

    bool isNonEmptyDirectory(const std::filesystem::path& path) {
        return std::filesystem::exists(path) &&
            std::filesystem::is_directory(path) &&
            !std::filesystem::is_empty(path);
    }

    std::ifstream openFile(const std::filesystem::path& path, const std::string& fileName) {
        const auto filePath = path / fileName;

        std::ifstream file(filePath);
        if (!file.is_open()) {
            throw std::runtime_error("Could not open file " + filePath.string());
        }

        return file;
    }

    void checkJobPath(const std::filesystem::path& dataPath, const std::string& jobId) {
        if (!exists(dataPath / jobId)) {
            throw std::runtime_error("Job: " + jobId + " does not exist in the given path: " + dataPath.string());
        }
    }

    void checkJobDataAvailable(const std::filesystem::path& jobPath) {
        if (!exists(jobPath / GlobalVariables::jobDataFileName)) {
            throw std::runtime_error("No " + static_cast<std::string>(GlobalVariables::jobDataFileName) + " was found.");
        }
    }

    nlohmann::json loadJobDataFromFile(const std::filesystem::path& path) {

        checkJobDataAvailable(path);

        std::ifstream file{openFile(path, GlobalVariables::jobDataFileName)};

        nlohmann::json j;
        try {
            j = nlohmann::json::parse(file);
        } catch (const nlohmann::json::parse_error& e) {
            std::stringstream ss{};
            ss << "There is a problem with the job_data.json\n" << e.what();
            throw std::runtime_error(ss.str());
        }

        return j;
    }

    void saveJobDataToFile(const std::filesystem::path& jobPath,
                        Config::FileConfig& fileConfig,
                        std::vector<CamData>& camDatas) {
        // Create JSON object with all information on this job,
        // that includes the configured parameters in the config.toml and information about the job itself.
        std::cout << "\nWriting job_data.json\n";
        nlohmann::json j;
        j["openCv"] = CV_VERSION;
        j["NLOHMANN_JSON"] = std::format("{}.{}.{}",
                                      NLOHMANN_JSON_VERSION_MAJOR,
                                      NLOHMANN_JSON_VERSION_MINOR,
                                      NLOHMANN_JSON_VERSION_PATCH);
        j["config"] = fileConfig;
        j["cams"] = nlohmann::json::object();

        for (auto i{0}; i < camDatas.size(); ++i) {
            j["cams"]["cam_" + std::to_string(i)] = camDatas[i].info;
        }

        // Save JSON to a file.
        std::ofstream file(jobPath / GlobalVariables::jobDataFileName);
        file << j.dump(4);
    }

    bool askYesNo() {
        char response;

        while (true) {
            std::cin >> response;
            response = std::tolower(response);

            if (response != 'n' && response != 'y') {
                std::cout << "Please enter 'y' or 'n': ";
            } else if (response == 'y') {
                return true;
            } else if (response == 'n') {
                return false;
            }
        }
    }
} // YACCP::Utility
