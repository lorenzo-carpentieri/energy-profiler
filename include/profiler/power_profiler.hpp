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
            PowerProfiler(int device_id,
                        int sampling_rate_ms,
                        const std::string& output_file);

            ~PowerProfiler();

            void start();
            void stop();

            data_types::energy_t get_device_energy() const; // energy consumed by the device with device_id in uj
            data_types::power_trace_t get_power_execution_data() const;  // return an std::vector containing the tuple (timestamp, power)
        
        private:
            struct Impl;                 // opaque PIMPL
            std::unique_ptr<Impl> impl_;                 // std::unique_ptr<Impl>
            data_types::timestamp_t get_timestamp() const;
            data_types::power_trace_t power_trace_data;
    };
}
