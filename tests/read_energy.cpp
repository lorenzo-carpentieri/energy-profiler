#include "profiler/power_profiler.hpp"
#include <sycl/sycl.hpp>
#include <vector>

#include <filesystem>
#define T float
#define N 8192
#define NUM_ITER (1 << 28) // 2^n

std::filesystem::path get_executable_dir() {
    char buffer[PATH_MAX];
    ssize_t len = readlink("/proc/self/exe", buffer, sizeof(buffer) - 1);
    if (len == -1) {
        throw std::runtime_error("Failed to get executable path");
    }
    buffer[len] = '\0';
    return std::filesystem::path(buffer).parent_path();
}

int main(){
    std::vector<T> a_vec(N, 1.0f);
    std::vector<T> b_vec(N, 2.0f);
    std::vector<T> c_vec(N, 0.0f);
    
    auto log_dir = get_executable_dir() / ".." / ".." / "logs"; // create logs directory relative to executable
    
    profiler::PowerProfiler profiler(0, 10, log_dir / "power_log.txt");
    profiler.start();
    

    sycl::queue q(sycl::gpu_selector_v);
    {
        sycl::buffer<T, 1> a_buf(a_vec);
        sycl::buffer<T, 1> b_buf(b_vec);
        sycl::buffer<T, 1> c_buf(c_vec);

        q.submit([&](sycl::handler& cgh){
            auto a_acc = a_buf.get_access<sycl::access::mode::read>(cgh);
            auto b_acc = b_buf.get_access<sycl::access::mode::read>(cgh);
            auto c_acc = c_buf.get_access<sycl::access::mode::read_write>(cgh);

            cgh.parallel_for<class vec_add>(sycl::range<1>(N), [=](sycl::id<1> idx){
                for (int i = 0; i < NUM_ITER; i++)
                    c_acc[idx] = a_acc[idx] + b_acc[idx];
            });
        });
        q.wait();
        sycl::host_accessor c_host_acc(c_buf, sycl::read_only);
        std::cout<< c_host_acc[0] +c_host_acc[c_vec.size()-1] <<std::endl;

    }

    profiler.stop();


    std::cout<< "Total energy [j]: " << profiler.get_device_energy() << std::endl;

    return 0;
}