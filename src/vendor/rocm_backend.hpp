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
        void initialize(uint32_t  dev_id) override{

            rsmi_init(RSMI_INIT_FLAG_ALL_GPUS);
            device_ = dev_id;
            // rsmi_dev_pci_id_get(dev_id, &device_);		
            // std::cerr << "Device id: "  << dev_id << " PCI dev id: " << device_ <<std::endl;
        }
        void shutdown() override{
            rsmi_shut_down();
        }

                // read current power in microwatts for primary die only
        profiler::data_types::power_t read_power() override {
            // If the current device is a secondary die (odd index), return 0
            if (device_ % 2 != 0) {
                // Optional: log once that secondary dies are skipped
                // std::cerr << "Device " << device_ << " is a secondary die, skipping power read." << std::endl;
                return 0;
            }

            profiler::data_types::power_t power_uw = 0;
            RSMI_POWER_TYPE power_type = RSMI_INVALID_POWER;

            rsmi_status_t ret =
                rsmi_dev_power_get(
                    static_cast<uint32_t>(device_),
                    &power_uw,
                    &power_type);

            if (ret != RSMI_STATUS_SUCCESS) {
                const char* status_string = nullptr;
                rsmi_status_string(ret, &status_string);

                std::cerr << "Power query failed for device " << device_ << ": "
                        << (status_string ? status_string : "Unknown error")
                        << std::endl;
                return 0;
            }

            return power_uw; // already in microwatts
        }
        
        profiler::data_types::energy_t read_energy() override{
            if (device_ % 2 != 0) {
                // Optional: log once that secondary dies are skipped
                // std::cerr << "Device " << device_ << " is a secondary die, skipping power read." << std::endl;
                return 0;
            }
            profiler::data_types::energy_t total_energy=0;
            float counter_resolution=0; // the energy coounter incremente in steps of minimum 15.3 uj
            uint64_t timestamp=0; 
            // TODO: add support for other AMD GPUs
            check(rsmi_dev_energy_count_get(static_cast<uint32_t>(device_), &total_energy, &counter_resolution, &timestamp), "rsmi_dev_energy_count_get"); // only works on AMD >= MI250X 
    	    
            // std::cerr << "Energy read: " << total_energy << " uj, resolution: " << counter_resolution << " uj, timestamp: " << timestamp << std::endl;
            return static_cast<profiler::data_types::energy_t>(total_energy); // in microjoules
        }

    private:
        uint32_t  device_;

    };
} // namespace nvidia
