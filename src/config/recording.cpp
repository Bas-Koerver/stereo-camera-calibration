#include "recording.hpp"

#include <stdexcept>

#include "../recoding/recorders/camera_worker.hpp"

namespace YACCP::Config {
    WorkerTypes stringToWorkerType(const std::string& worker) {
        std::unordered_map<std::string, WorkerTypes> map{
            {"prophesee", WorkerTypes::prophesee},
            {"basler", WorkerTypes::basler}
        };
        if (const auto it = map.find(worker); it != map.end()) {
            return it->second;
        }
        throw std::runtime_error("Unknown worker type: " + worker);
    }

    void parseRecordingConfig(const toml::table& tbl, RecordingConfig& config) {
        // [recording] configuration variables.
        const toml::node_view recording{tbl["recording"]};
        if (const auto subTbl{recording.as_table()}; !subTbl)
            throw std::runtime_error("Missing [recording] table");

        const auto fpsOpt{recording["fps"].value<int>()};
        const auto accumulationTimeOpt{recording["accumulationTime"].value<int>()};

        if (fpsOpt && !accumulationTimeOpt) {
            config.fps = *fpsOpt;
            config.accumulationTime = static_cast<int>(std::floor(1. / config.fps * 1e6));
        } else if (!fpsOpt && accumulationTimeOpt) {
            config.fps = static_cast<int>(std::floor(1e6 / *accumulationTimeOpt));
            config.accumulationTime = *accumulationTimeOpt;
        } else if (fpsOpt && accumulationTimeOpt) {
            config.fps = *fpsOpt;
            config.accumulationTime = *accumulationTimeOpt;
        } else {
            config.fps = 30;
            config.accumulationTime = static_cast<int>(std::floor(1. / 30 * 1e6));
        }

        if (const auto *slaveWorkerArray{recording["slave_workers"].as_array()}) {
            for (std::size_t i{0} ; i < slaveWorkerArray->size() ; ++i) {
                const auto element{(*slaveWorkerArray)[i].value<std::string>()};

                if (!element)
                    throw std::runtime_error("slave_worker[" + std::to_string(i) + "] must be a string");

                config.slaveWorkers.emplace_back(stringToWorkerType(*element));
            }
        }

        const auto &masterWorker {recording["master_worker"].as_string()};
        if (!masterWorker) throw std::runtime_error("Missing master worker");

        config.masterWorker = stringToWorkerType(masterWorker->get());

        if (const auto *camPlacementArray{recording["cam_placement"].as_array()}) {
            for (std::size_t i{0} ; i < camPlacementArray->size() ; ++i) {
                const auto element{(*camPlacementArray)[i].value<int>()};

                if (!element)
                    throw std::runtime_error("cam_placement[" + std::to_string(i) + "] must be an integer");

                config.camPlacement.emplace_back(*element);
            }
        }

        const auto masterCameraOpt = recording["master_camera"].value<int>();
        if (!masterCameraOpt)
            throw std::runtime_error("recording.master_camera is required");
        config.masterCamera = *masterCameraOpt;

        // [prophesee] worker
        const toml::node_view prophesee{recording["workers"]["prophesee"]};
        config.prophesee.saveEventfile = prophesee["save_eventfile"].value_or(false);

        if (const auto *camUuidArray{prophesee["cam_uuid"].as_array()}) {
            for (std::size_t i{0} ; i < camUuidArray->size() ; ++i) {
                const auto element{(*camUuidArray)[i].value<std::string>()};

                if (!element)
                    throw std::runtime_error("prophesee.cam_uuid[" + std::to_string(i) + "] must be a string");

                config.prophesee.camUuid.emplace_back(*element);
            }
        }

        // [basler] worker
        const toml::node_view basler{recording["workers"]["basler"]};
        if (const auto *camUuidArray{basler["cam_uuid"].as_array()}) {
            for (std::size_t i{0} ; i < camUuidArray->size() ; ++i) {
                const auto element{(*camUuidArray)[i].value<std::string>()};

                if (!element)
                    throw std::runtime_error("prophesee.cam_uuid[" + std::to_string(i) + "] must be a string");

                config.basler.camUuid.emplace_back(*element);
            }
        }

    }
} // YACCP::Config
