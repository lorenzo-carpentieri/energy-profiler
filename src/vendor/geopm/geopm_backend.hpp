#pragma once
#include "backend/gpu_energy_backend.hpp"
#include "backend/cpu_energy_backend.hpp"
#include <geopm/PlatformIO.hpp>
#include <geopm/PlatformTopo.hpp>
#include <vector>
#include <stdexcept>



namespace geopm_backend {
    struct geopm_device_handle {
            uint32_t id;
            geopm_domain_e domain;  // GEOPM_DOMAIN_GPU or GEOPM_DOMAIN_GPU_CHIP
    };

    class GEOPMIntelGPU final : public GPUEnergyBackend {
    public:
        void initialize(uint32_t dev_id) override{
            handle_ = geopm_device_handle{dev_id, geopm_domain_e::GEOPM_DOMAIN_GPU_CHIP};
        }   
        void shutdown() override{
        }

        profiler::data_types::power_t read_power() override{
          double power_val = geopm::platform_io().read_signal("LEVELZERO::GPU_CORE_POWER", handle_.domain, handle_.id); 
          return static_cast<profiler::data_types::power_t>(power_val*1e+6); // convert from Watts to microwatts
        }
        
        profiler::data_types::energy_t read_energy() override{
            double energy_val = geopm::platform_io().read_signal("LEVELZERO::GPU_CORE_ENERGY", handle_.domain, handle_.id); 
            return static_cast<profiler::data_types::energy_t>(energy_val*1e+6); // convert from Joules to microjoules
        }

    private:
        geopm_device_handle handle_;
    };

    class GEOPMCpu final : public CPUEnergyBackend {
    public:
        void initialize(uint32_t cpu_socket) override{
            handle_ = geopm_device_handle{cpu_socket, geopm_domain_e::GEOPM_DOMAIN_CPU};
        }   
        void shutdown() override{
        }

        profiler::data_types::power_t read_power() override{
          double power_val = geopm::platform_io().read_signal("CPU_POWER", handle_.domain, handle_.id); 
          return static_cast<profiler::data_types::power_t>(power_val*1e+6); // convert from Watts to microwatts
        }
        
        profiler::data_types::energy_t read_energy() override{
            double energy_val = geopm::platform_io().read_signal("CPU_ENERGY", handle_.domain, handle_.id); 
            return static_cast<profiler::data_types::energy_t>(energy_val*1e+6); // convert from Joules to microjoules
        }

    private:
        geopm_device_handle handle_;
    };
} // namespace geopm
