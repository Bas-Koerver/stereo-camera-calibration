#ifndef YACCP_RECORDING_RECORDERS_PROPHESEE_CAM_WORKER_HPP
#define YACCP_RECORDING_RECORDERS_PROPHESEE_CAM_WORKER_HPP
#include "camera_worker.hpp"


namespace YACCP {
    struct CamData;

    class PropheseeCamWorker final : public CameraWorker {
    public:
        PropheseeCamWorker(std::stop_source stopSource,
                           std::vector<CamData>& camDatas,
                           Config::RecordingConfig& recordingConfig,
                           const Config::Prophesee& configBackend,
                           int index,
                           const std::filesystem::path& jobPath);

        static void listAvailableSources();

        void start() override;

    private:
        const Config::Prophesee& configBackend_;
        int requestedFrame_{-1};
        int frameIndex_{};;
    };
} // YACCP
#endif //YACCP_RECORDING_RECORDERS_PROPHESEE_CAM_WORKER_HPP