#include "backend_factory.hpp"
#if defined(USE_NVML)
#include "vendor/nvml_backend.hpp"
#elif defined(USE_ROCM)
#include "vendor/rocm_backend.hpp"
#elif defined(USE_LEVEL_ZERO)
#include "vendor/lz_backend.hpp"
#elif defined(USE_GEOPM)
#include "vendor/geopm_backend.hpp"
#endif

#if defined(USE_RAPL)
#include "vendor/rapl_backend.hpp"
#endif

namespace gpu_backend {
    std::unique_ptr<GPUEnergyBackend> create_backend() {
    #if defined(USE_NVML)
        return std::make_unique<nvidia::NvmlBackend>();
    #elif defined(USE_ROCM)
        return std::make_unique<amd::RocmBackend>();
    #elif defined(USE_LEVEL_ZERO)
        return std::make_unique<intel::LevelZeroBackend>();
    #elif defined(USE_GEOPM)
        return std::make_unique<geopm_backend::GEOPMBackend>();
    #else
    #error "No GPU energy backend defined"
    #endif
    }
}


namespace cpu_backend {
    std::unique_ptr<CPUEnergyBackend> create_backend() {
    #if defined(USE_RAPL)
        return std::make_unique<rapl::RAPLBackend>();
    #else
    #warning "No CPU energy backend defined"
    #endif
    }
}
