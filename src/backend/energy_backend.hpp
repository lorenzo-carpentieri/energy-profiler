#pragma once
#include <cstdint>
#include "profiler/data_types.hpp"

// This is the interface used to abstract energy backends from different libraries such as NVML, ROCm SMI, Level Zero etc...
class EnergyBackend {
public:
    virtual ~EnergyBackend() = default;
    // initialize the backend library
    virtual void initialize(int device_id) = 0;
    // clear the backend library
    virtual void shutdown() = 0;
    // Read the current power consumption in watts
    virtual profiler::data_types::power_t read_power() = 0;
    // Read the total energy consumed in microjoules uj
    virtual profiler::data_types::energy_t read_energy() = 0;
};
