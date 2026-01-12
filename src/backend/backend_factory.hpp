#pragma once
#include <memory>
#include "energy_backend.hpp"
namespace backend {
    std::unique_ptr<EnergyBackend> create_backend();
}