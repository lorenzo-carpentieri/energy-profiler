#pragma once
#include <memory>
#include "gpu_energy_backend.hpp"
#include "cpu_energy_backend.hpp"

namespace gpu_backend {
    std::unique_ptr<GPUEnergyBackend> create_backend();
}

namespace cpu_backend {
    std::unique_ptr<CPUEnergyBackend> create_backend();
}
