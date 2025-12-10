#ifndef STEREO_CAMERA_CALIBRATION_BASLER_RGB_RECORDER_H
#define STEREO_CAMERA_CALIBRATION_BASLER_RGB_RECORDER_H
#include "camera_worker.hpp"
#include <pylon/PylonIncludes.h>


namespace YACCP {
    class BaslerRGBWorker : public CameraWorker {
    public:
        BaslerRGBWorker(std::stop_source stopSource,
                        CamData &camData,
                        int fps,
                        int id,
                        std::string camId = {});

        void listAvailableSources() override;

        void start() override;

        ~BaslerRGBWorker() override;

    private:
        void setPixelFormat(GenApi::INodeMap &nodeMap);

        std::tuple<int, int> getSetNodeMapParameters(GenApi::INodeMap &nodeMap);
    };
}

#endif //STEREO_CAMERA_CALIBRATION_BASLER_RGB_RECORDER_H
