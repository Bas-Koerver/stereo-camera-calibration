#ifndef STEREO_CAMERA_CALIBRATION_PROPHESEE_DVS_RECORDER_H
#define STEREO_CAMERA_CALIBRATION_PROPHESEE_DVS_RECORDER_H
#include "camera_worker.hpp"


namespace YACCP {
    class PropheseeDVSWorker : public CameraWorker {
    public:
        PropheseeDVSWorker(std::stop_source stopSource,
                           CamData &camData,
                           int fps,
                           int id,
                           int accumulation_time,
                           std::string camId = {});

        void listAvailableSources() override;

        void start() override;

    private:
        const std::uint16_t accumulationTime_;
    };
}
#endif //STEREO_CAMERA_CALIBRATION_PROPHESEE_DVS_RECORDER_H
