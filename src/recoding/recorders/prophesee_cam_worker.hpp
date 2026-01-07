#ifndef YACCP_RECORDING_RECORDERS_PROPHESEE_CAM_WORKER_HPP
#define YACCP_RECORDING_RECORDERS_PROPHESEE_CAM_WORKER_HPP
#include "camera_worker.hpp"


namespace YACCP {
    struct CamData;

    class PropheseeCamWorker final : public CameraWorker {
    public:
        PropheseeCamWorker(std::stop_source stopSource,
                           std::vector<CamData> &camDatas,
                           int fps,
                           int id,
                           int accumulationTime,
                           int fallingEdgePolarity,
                           const std::filesystem::path &outputPath,
                           std::string camId = {});

        void listAvailableSources() override;

        void start() override;

    private:
        const std::uint16_t accumulationTime_;
        int requestedFrame_{-1};
        int frameIndex_{};
        int fallingEdgePolarity_{};
    };
} // YACCP
#endif //YACCP_RECORDING_RECORDERS_PROPHESEE_CAM_WORKER_HPP