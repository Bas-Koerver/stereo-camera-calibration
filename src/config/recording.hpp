#ifndef YACCP_SRC_CONFIG_RECORDING_HPP
#define YACCP_SRC_CONFIG_RECORDING_HPP
#include <variant>

#include <nlohmann/json.hpp>

#include <toml++/toml.hpp>

namespace YACCP::Config {
    /**
    * @brief Simple enum to represent different camera worker types.
    */
    enum class WorkerTypes {
        prophesee,
        basler,
    };

    inline std::unordered_map<std::string, WorkerTypes> workerTypesMap{
        {"prophesee", WorkerTypes::prophesee},
        {"basler", WorkerTypes::basler}
    };

    struct Basler {
    };

    struct Prophesee {
        int accumulationTime{};
        bool saveEventFile{};
        int fallingEdgePolarity{};
    };

    using ConfigBackend = std::variant<Basler, Prophesee>;

    struct RecordingConfig {
        struct Worker {
            int placement{};
            std::string camUuid{};

            ConfigBackend configBackend;
        };

        int fps{};
        int masterWorker{};
        std::vector<Worker> workers{};
    };

    // Logic for loading config structs to JSON and loading it from a JSON back to the appropriate structs.
    inline void to_json(nlohmann::json& j, const Prophesee& p) {
        j = {
            {"accumulationTime", p.accumulationTime},
            {"fallingEdgePolarity", p.fallingEdgePolarity},
            {"saveEventFile", p.saveEventFile}
        };
    }


    inline void from_json(const nlohmann::json& j, Prophesee& p) {
        j.at("accumulationTime").get_to(p.accumulationTime);
        j.at("fallingEdgePolarity").get_to(p.fallingEdgePolarity);
        j.at("saveEventFile").get_to(p.saveEventFile);
    }


    inline void to_json(nlohmann::json& j, const RecordingConfig::Worker& w) {
        j = {
            {"placement", w.placement},
            {"camUuid", w.camUuid}
        };

        std::visit([&](const auto& workerBackend) {
                       using T = std::decay_t<decltype(workerBackend)>;

                       if constexpr (std::is_same_v<T, Basler>) {
                           j["type"] = "basler";
                       } else if constexpr (
                           std::is_same_v<T, Prophesee>) {
                           j["type"] = "prophesee";
                           nlohmann::json bj(workerBackend);
                           j.update(bj);
                       }
                   },
                   w.configBackend);
    }


    inline void from_json(const nlohmann::json& j, RecordingConfig::Worker& w) {
        j.at("placement").get_to(w.placement);
        j.at("camUuid").get_to(w.camUuid);

        // Discriminator
        const std::string type = j.at("type");

        // Backend
        if (type == "basler") {
            w.configBackend = Basler{};
        } else if (type == "prophesee") {
            w.configBackend = j.get<Prophesee>();
        } else {
            throw std::runtime_error("Unknown worker type: " + type);
        }
    }


    inline void to_json(nlohmann::json& j, const RecordingConfig& r) {
        j = {
            {"fps", r.fps},
            {"masterWorker", r.masterWorker},
            {"workers", r.workers},
        };
    }


    inline void from_json(const nlohmann::json& j, RecordingConfig& r) {
        j.at("fps").get_to(r.fps);
        j.at("masterWorker").get_to(r.masterWorker);
        r.workers = j.at("workers").get<std::vector<RecordingConfig::Worker> >();
    }


    WorkerTypes stringToWorkerType(std::string worker);

    std::string workerTypeToString(WorkerTypes workerType);

    void parseRecordingConfig(const toml::table& tbl, RecordingConfig& config);
} // YACCP::Config

#endif //YACCP_SRC_CONFIG_RECORDING_HPP