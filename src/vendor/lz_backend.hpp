#pragma once
#include "backend/energy_backend.hpp"
#include <level_zero/ze_api.h>
#include <level_zero/zes_api.h>
#include <vector>
namespace intel {
    class LevelZeroBackend final : public EnergyBackend {
    public:
        void initialize(int dev_id) override{
            zeInit(0); 
            device_ = get_devices()[dev_id];
        }
        void shutdown() override{
        }
        // read current power in microwatts: level zero do not support instantaneous power reading, so we return energy instead
        double read_power() override{
            zes_pwr_handle_t hPwr;
            zesDeviceGetCardPowerDomain(device_, &hPwr);
            zes_power_energy_counter_t counter;
            zesPowerGetEnergyCounter(hPwr, &counter);
            return  counter.energy; // in micro watts
        }
        
        uint64_t read_energy() override{
            zes_pwr_handle_t hPwr;
            zesDeviceGetCardPowerDomain(device_, &hPwr);
            zes_power_energy_counter_t counter;
            zesPowerGetEnergyCounter(hPwr, &counter);
            return  counter.energy; // in microjoules
        }

    private:
        ze_device_handle_t device_;

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
