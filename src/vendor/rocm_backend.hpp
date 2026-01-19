#pragma once
#include "backend/energy_backend.hpp"
#include <rocm_smi/rocm_smi.h>
#include <iostream>

namespace amd {
    inline void check(rsmi_status_t status, const char* call_name)
    {
        if (status != RSMI_STATUS_SUCCESS) {
            const char* err_str = nullptr;
            rsmi_status_string(status, &err_str);

            throw std::runtime_error(
                std::string(call_name) + " failed: " +
                (err_str ? err_str : "unknown RSMI error"));
        }
    }


    class RocmBackend final : public EnergyBackend {
    public:
        void initialize(int dev_id) override{
            rsmi_init(0);
            rsmi_dev_id_get(dev_id, &device_);		
        }
        void shutdown() override{
            rsmi_shut_down();
        }

        // read current power in microwatts
        profiler::data_types::power_t read_power() override{
            profiler::data_types::power_t power_uw;
            rsmi_dev_power_ave_get(device_, 0, &power_uw); // microwatts
            return static_cast<double>(power_uw); // convert mW to uW
        }
        
        profiler::data_types::energy_t read_energy() override{
            unsigned long long energy_uj=0;
            profiler::data_types::energy_t total_energy=0;
            float counter_resolution=0;
            uint64_t timestamp=0;
            // TODO: add support for other AMD GPUs
            check(rsmi_dev_energy_count_get(device_, &total_energy, &counter_resolution, &timestamp), "rsmi_dev_energy_count_get"); // only works on AMD >= MI250X 
    	    std::cout << "Energy read: " << total_energy << " uj, resolution: " << counter_resolution << " uj, timestamp: " << timestamp << std::endl;
            return static_cast<profiler::data_types::energy_t>(total_energy * counter_resolution); // in microjoules
        }

    private:
        uint16_t device_;

    };
} // namespace nvidia
