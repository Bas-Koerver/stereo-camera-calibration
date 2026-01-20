#ifndef YACCP_RECORDING_RECORDERS_BASLER_CAM_WORKER_HPP
#define YACCP_RECORDING_RECORDERS_BASLER_CAM_WORKER_HPP
#include "camera_worker.hpp"
// #include "../../config/orchestrator.hpp"

#include <pylon/PylonIncludes.h>



namespace YACCP {
    class BaslerCamWorker final : public CameraWorker {
    public:
        /**
        * @copydoc YACCP::CameraWorker::CameraWorker
        */
        BaslerCamWorker(std::stop_source stopSource,
                        std::vector<CamData>& camDatas,
                        Config::RecordingConfig& recordingConfig,
                        const Config::Basler& configBackend,
                        int index,
                        const std::filesystem::path& jobPath);

        static void listAvailableSources();

        void start() override;

        ~BaslerCamWorker() override;

    private:
        const Config::Basler& configBackend_;
        int requestedFrame_{1}; // Start from frame 1
        int detectionInterval_{2}; // seconds

        void setPixelFormat(GenApi::INodeMap& nodeMap);

        [[nodiscard]] std::tuple<int, int> getSetNodeMapParameters(GenApi::INodeMap& nodeMap);
    };
} // YACCP

#endif //YACCP_RECORDING_RECORDERS_BASLER_CAM_WORKER_HPP