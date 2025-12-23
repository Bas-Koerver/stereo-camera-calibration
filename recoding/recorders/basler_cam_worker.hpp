#ifndef STEREO_CAMERA_CALIBRATION_BASLER_RGB_RECORDER_H
#define STEREO_CAMERA_CALIBRATION_BASLER_RGB_RECORDER_H
#include <pylon/PylonIncludes.h>

#include "camera_worker.hpp"


namespace YACCP {
    class BaslerCamWorker final : public CameraWorker {
    public:
        BaslerCamWorker(std::stop_source stopSource,
                        std::vector<CamData> &camDatas,
                        int fps,
                        int id,
                        const std::filesystem::path &outputPath,
                        std::string camId = {});

        void listAvailableSources() override;

        void start() override;

        ~BaslerCamWorker() override;

    private:
        int requestedFrame_{1}; // Start from frame 1
        int detectionInterval_{2}; // seconds

        void setPixelFormat(GenApi::INodeMap &nodeMap);

        [[nodiscard]] std::tuple<int, int> getSetNodeMapParameters(GenApi::INodeMap &nodeMap);
    };
} // YACCP

#endif //STEREO_CAMERA_CALIBRATION_BASLER_RGB_RECORDER_H