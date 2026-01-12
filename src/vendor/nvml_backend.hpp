#pragma once
#include "backend/energy_backend.hpp"
#include <nvml.h>

namespace nvidia {
    class NvmlBackend final : public EnergyBackend {
    public:
        void initialize(int dev_id) override{
            nvmlInit();
            nvmlDeviceGetHandleByIndex(dev_id, &device_);
        }
        void shutdown() override{
            nvmlShutdown();
        }
        // read current power in microwatts
        double read_power() override{
            unsigned int power_mw;
            nvmlDeviceGetPowerUsage(device_, &power_mw);
            return static_cast<double>(power_mw) * 1000.0; // convert mW to uW
        }
        
        uint64_t read_energy() override{
            unsigned long long energy_uj;
            nvmlDeviceGetTotalEnergyConsumption(device_, &energy_uj);
            return static_cast<uint64_t>(energy_uj); // in microjoules
        }

    private:
        nvmlDevice_t device_;

    };
} // namespace nvidia
