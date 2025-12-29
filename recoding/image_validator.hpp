#ifndef STEREO_CAMERA_CALIBRATION_IMAGE_VALIDATOR_HPP
#define STEREO_CAMERA_CALIBRATION_IMAGE_VALIDATOR_HPP
#include <filesystem>

#include <metavision/sdk/core/utils/frame_composer.h>

#include <opencv2/core/mat.hpp>

namespace YACCP {
    class ImageValidator {
    public:
        ImageValidator(int resolutionWidth,
                       int resolutionHeight,
                       const std::filesystem::path &dataPath);

        void listJobs() const;

        void validateImages(const std::string &jobName);

        [[nodiscard]] static bool isNonEmptyDirectory(const std::filesystem::path &path);

    private:
        int resolutionWidth_;
        int resolutionHeight_;
        const std::filesystem::path &dataPath_;
        std::filesystem::path jobPath_{};
        int currentFileIndex_{0};
        std::vector<int> indexesToDiscard_;

        void updateSubimages(Metavision::FrameComposer &frameComposer,
                             const std::vector<std::filesystem::path> &files,
                             const std::vector<std::filesystem::path> &cams,
                             const std::vector<int> &camRefs) const;
    };
} // YACCP

#endif //STEREO_CAMERA_CALIBRATION_IMAGE_VALIDATOR_HPP
