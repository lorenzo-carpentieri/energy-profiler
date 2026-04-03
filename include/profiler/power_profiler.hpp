#pragma once
#include <string>
#include <cstdint>
#include <memory>
#include <vector>
#include <tuple>
#include "./data_types.hpp"

namespace profiler{
    class PowerProfiler {
        public:
            PowerProfiler(int device_id, int num_sockets,
                        int sampling_rate_ms);

            ~PowerProfiler();

            void start();
            void stop();

            data_types::energy_t get_device_energy() const; // energy consumed by the device with device_id in uj
            data_types::energy_t get_host_energy() const; // energy consumed by the host in uj

            data_types::power_trace_t get_power_execution_data() const;  // return an std::vector containing the tuple (timestamp, power) for the device
            data_types::power_trace_t get_host_power_trace() const;  // return an std::vector containing the tuple (timestamp, power) for the host 

        private:
            struct Impl;                 // opaque PIMPL
            std::unique_ptr<Impl> impl_;                 // std::unique_ptr<Impl>
            data_types::timestamp_t get_timestamp() const;
            data_types::power_trace_t power_trace_data;
            data_types::power_trace_t host_power_trace;

            data_types::energy_t compute_energy(const data_types::power_trace_t& trace) const;
    };
}
