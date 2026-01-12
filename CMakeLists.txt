cmake_minimum_required(VERSION 3.20)
project(power_profiler LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ===============================
# Options
# ===============================
option(USE_NVML "Enable NVIDIA NVML backend" OFF)
option(USE_ROCM "Enable AMD ROCm backend" OFF)
option(USE_LEVEL_ZERO "Enable Intel Level Zero backend" OFF)
option(BUILD_TESTS "Compile test" OFF)


# ===============================
# Library
# ===============================
add_library(power_profiler STATIC
    src/profiler/power_profiler.cpp
    src/backend/backend_factory.cpp
)

target_include_directories(power_profiler
    PUBLIC
        include
    PRIVATE
        src
)


# ===============================
# Backend selection
# ===============================
if(USE_NVML)
    target_compile_definitions(power_profiler PRIVATE USE_NVML)
    target_link_libraries(power_profiler PRIVATE nvidia-ml)
endif()

if(USE_ROCM)
    target_compile_definitions(power_profiler PRIVATE USE_ROCM)
endif()

if(USE_LEVEL_ZERO)
    target_compile_definitions(power_profiler PRIVATE USE_LEVEL_ZERO)
endif()

# ===============================
# Tests
# ===============================
if(BUILD_TESTS)
    add_subdirectory(tests)
endif()
