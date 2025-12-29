#ifndef STEREO_CAMERA_CALIBRATION_CAMERA_WORKER_HPP
#define STEREO_CAMERA_CALIBRATION_CAMERA_WORKER_HPP
#include <filesystem>
#include <mutex>
#include <readerwriterqueue.h>

#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>

#include <opencv2/core/mat.hpp>
#include <opencv2/objdetect/aruco_detector.hpp>
#include <opencv2/objdetect/aruco_dictionary.hpp>
#include <opencv2/objdetect/charuco_detector.hpp>


namespace YACCP {
    struct CamData;
    struct VerifyTask;
    /**
     * @brief Simple enum to represent different camera worker types.
     */
    enum class WorkerTypes {
        prophesee,
        basler,
    };

    /**
     * @brief Parent class for camera workers.
     */
    class CameraWorker {
    public:
        /**
         * @brief Camera worker constructor.
         *
         * @param stopSource Stop source to control the program execution.
         * @param camDatas Reference to the camera data struct.
         * @param fps Frames per second to capture at.
         * @param id ID of the camera worker this must be the index of the correct CamData from camDatas.
         * @param outputPath
         * @param camId A user can give a specific camera ID to connect to.
         */
        CameraWorker(std::stop_source stopSource,
                     std::vector<CamData> &camDatas,
                     int fps,
                     int id,
                     std::filesystem::path outputPath,
                     std::string camId = {});

        /**
         * @brief Function to list available camera sources.
         */
        virtual void listAvailableSources();

        /**
         * @brief Function to start the camera worker.
         */
        virtual void start();

        virtual ~CameraWorker() = default;

    protected:
        std::stop_source stopSource_;
        std::stop_token stopToken_;
        std::vector<CamData> &camDatas_;
        CamData &camData_;
        const int fps_;
        const int id_;
        std::filesystem::path outputPath_;
        // A user can give a specific camera ID to connect to.
        std::string camId_;
    };
} // YACCP

#endif //STEREO_CAMERA_CALIBRATION_CAMERA_WORKER_HPP