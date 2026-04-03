#pragma once
#include "backend/cpu_energy_backend.hpp"
#include <vector>
#include <stdexcept>
#include <fstream>
#include <filesystem>
#include <iostream>
#include <chrono>
namespace rapl {
    constexpr auto POWERCAP_ROOT_DIR = "/sys/class/powercap";
    constexpr auto POWERCAP_ENERGY_FILE = "energy_uj";
    constexpr auto POWERCAP_UNCORE_NAME = "dram";
    constexpr auto POWERCAP_CORE_NAME = "core";
    constexpr auto POWERCAP_PACKAGE_NAME = "package";

    class RAPLBackend final : public CPUEnergyBackend {
    public:
        void initialize(uint32_t num_sockets) override{
            rapl_pkgs = num_sockets;
        }
        
        void shutdown() override{
        }
        // read current power in microwatts: level zero do not support instantaneous power reading, so we return energy instead
        profiler::data_types::power_t read_power() override{
            if(prev_energy == 0 || prev_timestamp == 0) {
                prev_energy = read_energy();
                prev_timestamp = std::chrono::duration_cast<std::chrono::microseconds>(
                    std::chrono::high_resolution_clock::now().time_since_epoch()
                ).count();
                return 0;
            }
            else {
                profiler::data_types::energy_t current_energy = read_energy();
                profiler::data_types::timestamp_t current_timestamp = std::chrono::duration_cast<std::chrono::microseconds>(
                    std::chrono::high_resolution_clock::now().time_since_epoch()
                ).count();
                profiler::data_types::energy_t energy_diff = current_energy - prev_energy;
                profiler::data_types::timestamp_t time_diff = current_timestamp - prev_timestamp;
                prev_energy = current_energy;
                prev_timestamp = current_timestamp;
                if(time_diff == 0) {
                    return 0; // avoid division by zero
                }
                return (energy_diff * 1000000) / time_diff; // power in microwatts 
            }
            return 0; // TODO: implements power reading with RAPL
        }
        // The rapl interface define a differen package for each CPU socket
        profiler::data_types::energy_t read_energy() override{
            profiler::data_types::energy_t cpu_energy = 0;
            for(int i = 0; i < rapl_pkgs; i++) {
                profiler::data_types::energy_t pkg_energy = 0;

                std::string pkg = "intel-rapl:" + std::to_string(i); // Get the package name for the given host_id
                // if it's a multi-cpu architecture, we want to sum the energy of all the cpus
                std::string path = build_path(POWERCAP_ROOT_DIR, pkg, POWERCAP_ENERGY_FILE);
                std::ifstream file{path, std::ios::in};
                if (!file.is_open()) {
                    std::cerr << "host_profiler error: could not open energy register file" << std::endl;
                    pkg_energy = 0;
                } else {
                    file >> pkg_energy; // read the energy value in microjoules
                }
                cpu_energy += pkg_energy;
            }            
            return cpu_energy;
        }

    private:
        uint32_t rapl_pkgs;
        profiler::data_types::energy_t prev_energy = 0; // store the previous energy reading to compute the energy difference between two readings
        profiler::data_types::timestamp_t prev_timestamp = 0; // store the previous timestamp to compute the power difference between two readings

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
