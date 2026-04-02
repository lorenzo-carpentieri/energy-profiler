#pragma once
#include "backend/gpu_energy_backend.hpp"
#include <level_zero/ze_api.h>
#include <level_zero/zes_api.h>
#include <vector>
#include <stdexcept>
namespace intel {
    class LevelZeroBackend final : public GPUEnergyBackend {
    public:
        void initialize(uint32_t dev_id) override{
            zeInit(0); 
            device_ = get_devices()[dev_id];
            // Add a first energy value into energy trace so that the first read_power is not 0
            zes_pwr_handle_t hPwr;
            zesDeviceGetCardPowerDomain(device_, &hPwr);
            zes_power_energy_counter_t counter;
            zesPowerGetEnergyCounter(hPwr, &counter);
            energy_time_trace.push_back(counter);
        }   
        void shutdown() override{
        }
        // read current power in microwatts: level zero do not support instantaneous power reading but the power can be calculated as energy difference between two consecutive energy readings divided by the time interval between them. In this way we can have an estimation of the power consumption in the interval of time between two consecutive energy readings.
        profiler::data_types::power_t read_power() override{
            zes_pwr_handle_t hPwr;
            zesDeviceGetCardPowerDomain(device_, &hPwr);
            zes_power_energy_counter_t counter;
            zesPowerGetEnergyCounter(hPwr, &counter);
            energy_time_trace.push_back(counter);        
            auto& last = energy_time_trace[energy_time_trace.size() - 1];
            auto& prev = energy_time_trace[energy_time_trace.size() - 2];
            double dt = static_cast<double>(last.timestamp - prev.timestamp); // time interval between two consecutive energy samples in ms 
            double p = static_cast<double>(last.energy - prev.energy) / dt; // power in Watts
            return static_cast<profiler::data_types::power_t>(p*1e+6);
        }
        
        profiler::data_types::energy_t read_energy() override{
            zes_pwr_handle_t hPwr;
            zesDeviceGetCardPowerDomain(device_, &hPwr);
            zes_power_energy_counter_t counter;
            zesPowerGetEnergyCounter(hPwr, &counter);
            return  counter.energy; // in microjoules
        }

    private:
        ze_device_handle_t device_;
        std::vector<zes_power_energy_counter_t> energy_time_trace;

        // Helper function to get all Level Zero devices associated to Leve Zero drivers
        // TODO: check if this works correctly with multiple drivers installed 
        inline std::vector<ze_device_handle_t> get_devices() const {
            unsigned int drivers_count = 0;
            zeDriverGet(&drivers_count, nullptr);

            if (drivers_count < 1) {
                throw std::runtime_error{"power_profiler: intel backend error: could not get Level Zero drivers"};
            }
            std::vector<ze_driver_handle_t> drivers(drivers_count);
            zeDriverGet(&drivers_count, drivers.data());

            unsigned int total_devices_count = 0;
            unsigned int devices_per_driver = 0;

            for (unsigned i = 0; i < drivers.size(); i++) {
                zeDeviceGet(drivers[i], &devices_per_driver, nullptr);
                total_devices_count += devices_per_driver;
            }

            std::vector<ze_device_handle_t> devices(total_devices_count);
            for (unsigned i = 0, offset = 0; i < drivers.size(); i++) {
                zeDeviceGet(drivers[i], &devices_per_driver, &devices.data()[offset]);
                offset += devices_per_driver;
            }

            return devices;
        }



    };
} // namespace nvidia
