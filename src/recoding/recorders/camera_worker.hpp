#ifndef YACCP_SRC_RECORDING_RECORDERS_CAM_WORKER_HPP
#define YACCP_SRC_RECORDING_RECORDERS_CAM_WORKER_HPP
#include "../../config/recording.hpp"

#include <mutex>

namespace YACCP {
    struct CamData;
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
         * @param recordingConfig Frames per second to capture at.
         * @param index ID of the camera worker this must be the index of the correct CamData from camDatas.
         * @param jobPath
         */
        CameraWorker(std::stop_source stopSource,
                     std::vector<CamData>& camDatas,
                     Config::RecordingConfig& recordingConfig,
                     int index,
                     std::filesystem::path jobPath);

        /**
         * @brief Function to list available camera sources.
         */
        static void listAvailableSources();

        static void listJobs(const std::filesystem::path& dataPath);

        /**
         * @brief Function to start the camera worker.
         */
        virtual void start() = 0;

        virtual ~CameraWorker() = default;


    protected:
        std::stop_source stopSource_;
        std::stop_token stopToken_;
        std::vector<CamData>& camDatas_;
        CamData& camData_;
        Config::RecordingConfig& recordingConfig_;
        const int index_;
        std::filesystem::path jobPath_;
    };
} // YACCP

#endif //YACCP_SRC_RECORDING_RECORDERS_CAM_WORKER_HPP