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

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

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
    find_package(CUDAToolkit REQUIRED)
    target_compile_definitions(power_profiler PRIVATE USE_NVML)
    target_link_libraries(power_profiler PRIVATE CUDA::nvml)

endif()

if(USE_ROCM)
    target_compile_definitions(power_profiler PRIVATE USE_ROCM)
    list(APPEND CMAKE_PREFIX_PATH "/opt/rocm")
    find_package(rocm_smi REQUIRED)

    target_include_directories(power_profiler PRIVATE ${ROCM_SMI_INCLUDE_DIRS})

    target_link_libraries(power_profiler INTERFACE ${ROCM_SMI_LIBRARIES})
endif()

if(USE_LEVEL_ZERO)
    target_compile_definitions(power_profiler PRIVATE USE_LEVEL_ZERO)
    find_package(LevelZero REQUIRED)
	target_link_libraries(power_profiler INTERFACE LevelZero::LevelZero)
endif()

install(TARGETS power_profiler
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
)

install(DIRECTORY include/
    DESTINATION include
)


# ===============================
# Tests
# ===============================
if(BUILD_TESTS)
    add_subdirectory(tests)
endif()
