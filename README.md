# PowerProfiler
PowerProfiler implements a portable energy interface for power and energy monitoring on GPUs and CPUs from different vendors by using libraries such as NVML, ROCm SMI and Level Zero.

## Build PowerProfiler library
```
mkdir build
cd build
cmake  -DUSE_{NVML/ROCM/LEVEL_ZERO}=ON  -DCMAKE_INSTALL_PREFIX=<path_to_install_dir>..
make -j
```

## Build samples with SYCL
```
cmake  -DUSE_{NVML/ROCM/LEVEL_ZERO}=ON -DBUILD_TESTS=ON ..
make -j 
```

## Run tests
```
./tests/read_energy_test
```