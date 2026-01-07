#ifndef YACCP_RECORDING_RECORDERS_BASLER_CAM_WORKER_HPP
#define YACCP_RECORDING_RECORDERS_BASLER_CAM_WORKER_HPP
#include <pylon/PylonIncludes.h>

#include "camera_worker.hpp"


namespace YACCP {
    struct CamData;

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

#endif //YACCP_RECORDING_RECORDERS_BASLER_CAM_WORKER_HPP