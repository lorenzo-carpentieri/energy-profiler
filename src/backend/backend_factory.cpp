#include "backend_factory.hpp"
#if defined(USE_NVML)
#include "vendor/nvml_backend.hpp"
#elif defined(USE_ROCM)
#include "vendor/RocmBackend.hpp"
#elif defined(USE_LEVEL_ZERO)
#include "vendor/LevelZeroBackend.hpp"
#endif

namespace backend {
    std::unique_ptr<EnergyBackend> create_backend() {
    #if defined(USE_NVML)
        return std::make_unique<nvidia::NvmlBackend>();
    #elif defined(USE_ROCM)
        return std::make_unique<amd::RocmBackend>();
    #elif defined(USE_LEVEL_ZERO)
        return std::make_unique<intel::LevelZeroBackend>();
    #else
    #error "No backend defined"
    #endif
    }
}
