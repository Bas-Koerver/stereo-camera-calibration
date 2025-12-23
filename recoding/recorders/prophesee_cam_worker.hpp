#ifndef STEREO_CAMERA_CALIBRATION_PROPHESEE_DVS_RECORDER_H
#define STEREO_CAMERA_CALIBRATION_PROPHESEE_DVS_RECORDER_H
#include "camera_worker.hpp"


namespace YACCP {
    class PropheseeCamWorker final : public CameraWorker {
    public:
        PropheseeCamWorker(std::stop_source stopSource,
                           std::vector<CamData> &camDatas,
                           int fps,
                           int id,
                           int accumulation_time,
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
#endif //STEREO_CAMERA_CALIBRATION_PROPHESEE_DVS_RECORDER_H