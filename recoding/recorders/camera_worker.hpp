#ifndef STEREO_CAMERA_CALIBRATION_CAMERA_WORKER_HPP
#define STEREO_CAMERA_CALIBRATION_CAMERA_WORKER_HPP
#include <mutex>
#include <readerwriterqueue.h>
#include <opencv2/core/mat.hpp>


namespace YACCP {
    enum class WorkerTypes {
        propheseeDVS,
        baslerRGB,
    };

    struct FrameData {
        int id;
        cv::Mat frame;
    };

    struct CamData {
        // Thread information
        bool isMaster;
        bool isOpen{false};
        bool isRunning{false};
        int exitCode;
        // Camera information
        std::string camName;
        int width{};
        int height{};
        cv::Mat frame;
        std::mutex m;
        // Communication
        moodycamel::BlockingReaderWriterQueue<int> frameRequestQ{100};
        moodycamel::BlockingReaderWriterQueue<FrameData> frameVerifyQ{100};
    };

    class CameraWorker {
    public:
        CameraWorker(std::stop_source stopSource,
                     CamData &camData,
                     int fps,
                     int id,
                     std::string camId = {});

        virtual void listAvailableSources();

        virtual void start();

        virtual ~CameraWorker() = default;

    protected:
        std::stop_source stopSource_;
        std::stop_token stopToken_;
        CamData &camData_;
        const int fps_;
        const int id_;
        // A user can give a specific camera ID to connect to.
        std::string camId_;
    };
} // YACCP

#endif //STEREO_CAMERA_CALIBRATION_CAMERA_WORKER_HPP
