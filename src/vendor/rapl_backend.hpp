#pragma once
#include "backend/cpu_energy_backend.hpp"
#include <vector>
#include <stdexcept>
#include <fstream>
#include <filesystem>
#include <iostream>
namespace rapl {
    constexpr auto POWERCAP_ROOT_DIR = "/sys/class/powercap";
    constexpr auto POWERCAP_ENERGY_FILE = "energy_uj";
    constexpr auto POWERCAP_UNCORE_NAME = "dram";
    constexpr auto POWERCAP_CORE_NAME = "core";
    constexpr auto POWERCAP_PACKAGE_NAME = "package";

    class RAPLBackend final : public CPUEnergyBackend {
    public:
        void initialize(uint32_t host_id) override{
            rapl_pkg = host_id;
        }
        void shutdown() override{
        }
        // read current power in microwatts: level zero do not support instantaneous power reading, so we return energy instead
        profiler::data_types::power_t read_power() override{
            return 0; // TODO: implements power reading with RAPL
        }
        // The rapl interface define a differen package for each CPU socket
        profiler::data_types::energy_t read_energy() override{
            profiler::data_types::energy_t energy = 0;
            std::string pkg = "intel-rapl:" + std::to_string(rapl_pkg); // Get the package name for the given host_id
            // if it's a multi-cpu architecture, we want to sum the energy of all the cpus
            std::string path = build_path(POWERCAP_ROOT_DIR, pkg, POWERCAP_ENERGY_FILE);
            std::ifstream file{path, std::ios::in};
            if (!file.is_open()) {
                std::cerr << "host_profiler error: could not open energy register file" << std::endl;
                energy = 0;
            } else {
                file >> energy; // read the energy value in microjoules
            }
            
            return energy;
        }

    private:
        uint32_t rapl_pkg;
        // Build the path to the energy file for the given package and energy type
        template <typename... Args>
        std::string build_path(Args... args) {
            std::string path;
            // ((path += "/" + std::string(args)) + ...); // fold expression to concatenate the strings
            ((path += "/" + std::string(args)), ...); // correct fold with comma operator
            return path;
        }

    };
} // namespace rapl
