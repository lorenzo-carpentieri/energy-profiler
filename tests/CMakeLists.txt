cmake_minimum_required(VERSION 3.20)

# Inherit compiler, SYCL, and flags from parent

add_executable(read_energy_test
    read_energy.cpp
)

target_link_libraries(read_energy_test
    PRIVATE power_profiler
)

# Optional: enable warnings for test only
target_compile_options(read_energy_test PRIVATE -Wall -Wextra)

# Optional: label for ctest
add_test(NAME read_energy_test COMMAND read_energy_test)
