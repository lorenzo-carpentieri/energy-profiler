#pragma once
#include "backend/gpu_energy_backend.hpp"
#include <level_zero/zes_api.h>
#include <vector>
#include <stdexcept>
#include <iostream>
#include <cstdlib>
#include <string.h>
#define LZ_CHECK(call) do {                                        \
    ze_result_t r = (call);                                     \
    if (r != ZE_RESULT_SUCCESS) {                               \
        std::cerr << "ERROR: " << #call                         \
                  << " failed with code " << r << std::endl;    \
        std::exit(1);                                           \
    }                                                           \
} while (0)

namespace intel {
    class LevelZeroBackend final : public GPUEnergyBackend {
    public:
        void initialize(uint32_t dev_id) override{
            setenv("ZES_ENABLE_SYSMAN", "1", 1);
            LZ_CHECK(zesInit(0));

            // Extract device id and subdevice ID assuming two tile per device
            dev_id_ = dev_id / 2;
            subdev_id_ = dev_id % 2; 
            // Suppose that we have one driver per system
            driver = get_drivers()[0]; // One driver per system
            // Suppose that we have two tile and device_ store the whole gpu device
            auto devices = get_devices();
            if (dev_id_ >= devices.size()) {
                throw std::runtime_error("Level Zero Sysman device id is out of range");
            }
            device_ = devices[dev_id_]; 
           
            pwrCount = 0;
            // Check num. of power domains.
            LZ_CHECK(zesDeviceEnumPowerDomains(device_, &pwrCount, nullptr));
            if (pwrCount == 0) {
                std::cerr << "No power domains available.\n";
                return;
            }
            
            pwrHandles.resize(pwrCount);
            LZ_CHECK(zesDeviceEnumPowerDomains(device_, &pwrCount, pwrHandles.data()));
            
            // Iterate on power domains to find the power handle of the tile atttached to the device
            bool found_tile = false;
            for (uint32_t i = 0; i < pwrCount; ++i) {
            
                power_props.stype = ZES_STRUCTURE_TYPE_POWER_PROPERTIES;
                power_props.pNext = nullptr;
                // Read properties of this power domain.
                LZ_CHECK(zesPowerGetProperties(pwrHandles[i], &power_props));

                // std::cout << "   onSubdevice = "
                //         << (power_props.onSubdevice ? "true" : "false") << "\n";
                // std::cout << "   subdeviceId = " << power_props.subdeviceId << "\n";

                // Take the power handle attached to the subdevice
                if (power_props.onSubdevice) {
                    TilePowerInfo info={};
                    info.hPower = pwrHandles[i];
                    info.subdeviceId = power_props.subdeviceId;
                    if(subdev_id_ == info.subdeviceId) {
                        tile.hPower = info.hPower;
                        tile.subdeviceId = info.subdeviceId;
                        found_tile = true;
                    }
                } 
            }
            if (!found_tile) {
                throw std::runtime_error("No Level Zero Sysman power domain found for requested tile");
            }

            // Retive the first energy counter at the initialization time so that the first power values can be computed.
            zes_power_energy_counter_t counter;
            LZ_CHECK(zesPowerGetEnergyCounter(tile.hPower, &counter));
            energy_time_trace.push_back(counter);
            // std::cout << "First energy counter: " << counter.energy << " uj, timestamp: " << counter.timestamp << std::endl;
        }

        void shutdown() override{
        }
        // read current power in microwatts: level zero do not support instantaneous power reading but the power can be calculated as energy difference between two consecutive energy readings divided by the time interval between them. In this way we can have an estimation of the power consumption in the interval of time between two consecutive energy readings.
        profiler::data_types::power_t read_power() override{
            zes_power_energy_counter_t counter;
            LZ_CHECK(zesPowerGetEnergyCounter(tile.hPower, &counter));
            energy_time_trace.push_back(counter);

            auto& last = energy_time_trace[energy_time_trace.size() - 1];
            auto& prev = energy_time_trace[energy_time_trace.size() - 2];
            double dt = static_cast<double>(last.timestamp - prev.timestamp); // time interval between two consecutive energy samples in ms 
            if (dt <= 0.0) {
                return 0;
            }
            double p = static_cast<double>(last.energy - prev.energy) / dt; // power in Watts
            return static_cast<profiler::data_types::power_t>(p*1e+6);
        }
        
        profiler::data_types::energy_t read_energy() override{
            zes_power_energy_counter_t counter;
            LZ_CHECK(zesPowerGetEnergyCounter(tile.hPower, &counter));
            // std::cout << "Energy counter: " << counter.energy << " uj, timestamp: " << counter.timestamp << std::endl;

            return  counter.energy; // in microjoules
        }

    private:
        
        struct TilePowerInfo {
            zes_pwr_handle_t hPower;   // power-domain handle for one tile
            uint32_t subdeviceId;      // tile id: usually 0 or 1 on Max 1550
        };

        TilePowerInfo tile = {};
        zes_device_handle_t device_;
        std::vector<zes_power_energy_counter_t> energy_time_trace;
        zes_driver_handle_t driver;
        uint32_t dev_id_; // Number of full GPUs
        uint32_t subdev_id_; // 0 or 1 assuming max two subdevices
        zes_device_properties_t devProps = {};
        uint32_t pwrCount = 0; // Num. of power domain
        std::vector<zes_pwr_handle_t> pwrHandles; // Power domain handles
        zes_power_properties_t power_props = {};

        // Helper function to get all Level Zero devices associated to Leve Zero drivers
        // TODO: I assume that the device index passed to the function is the index of the device in flat mode.
        // So if I have 6 GPUs each with 2 tile the device id can be 0-11.
        inline std::vector<zes_device_handle_t> get_devices() const {
            // Get number of device on the system. zes is not affected by env variable like flat or composite
            uint32_t devCount = 0;
            LZ_CHECK(zesDeviceGet(driver, &devCount, nullptr));
            std::vector<zes_device_handle_t> devices(devCount);
            LZ_CHECK(zesDeviceGet(driver, &devCount, devices.data()));
            // std::cout << "Num devices: " << devCount << std::endl;
            return devices;
        }
        
        inline std::vector<zes_driver_handle_t> get_drivers() const {
            uint32_t driverCount = 0;
            LZ_CHECK(zesDriverGet(&driverCount, nullptr));
            if(driverCount == 0){
                std::cerr << "No driverr found on this system" << std::endl;
                return;
            }
            std::vector<zes_driver_handle_t> drivers(driverCount);
            LZ_CHECK(zesDriverGet(&driverCount, drivers.data()));
            return drivers;
        }

        


    };
} // namespace nvidia
