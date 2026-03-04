#include "profiler/power_profiler.hpp"
#include "backend/gpu_energy_backend.hpp"
#include "backend/cpu_energy_backend.hpp"
#include "backend/backend_factory.hpp"
#include <thread>
#include <atomic>
#include <fstream>
#include <chrono>
#include <iomanip>
#include <filesystem>
#include <algorithm>

namespace profiler{
    struct PowerProfiler::Impl {
        std::unique_ptr<GPUEnergyBackend> gpu_backend;
        std::unique_ptr<CPUEnergyBackend> cpu_backend;
        // PowercapReader host_energy;

        std::atomic<bool> running{false};
        std::thread worker;

        data_types::energy_t start_dev_energy = 0;
        data_types::energy_t end_dev_energy   = 0;

        data_types::energy_t start_host_energy = 0;
        data_types::energy_t end_host_energy   = 0;
        double total_energy_uj = 0.0f;
        int sampling_ms;
        std::ofstream log;
    };

    PowerProfiler::PowerProfiler(int  dev_id, int host_id,
                                int sampling_rate_ms)
        : impl_(new Impl) {

        impl_->gpu_backend = gpu_backend::create_backend();
        impl_->cpu_backend = cpu_backend::create_backend();
        impl_->gpu_backend->initialize(static_cast<uint32_t>(dev_id));
        impl_->cpu_backend->initialize(static_cast<uint32_t>(host_id));
        impl_->sampling_ms = sampling_rate_ms;

    }

    PowerProfiler::~PowerProfiler() {
        impl_->gpu_backend->shutdown(); // shutdown gpu_backend
        impl_->cpu_backend->shutdown(); // shutdown gpu_backend

        impl_.reset(); // destroy pointer to gpu_backend implementation
    }

    // TODO: Remove power file: timestamp and power will be stored in a vector of tuple 
    void PowerProfiler::start() {
        impl_->running = true;

        impl_->start_dev_energy = impl_->gpu_backend->read_energy(); // start energy in uj
        impl_->start_host_energy = impl_->cpu_backend->read_energy();
        impl_->worker = std::thread([this]() {
            uint64_t timestamp=0;
            while (impl_->running) {
                data_types::power_t power_uw = impl_->gpu_backend->read_power(); // read power in uw
                std::tuple<data_types::timestamp_t, data_types::power_t> power_tuple = std::make_tuple(timestamp, power_uw);
                power_trace_data.push_back(power_tuple); // Create (timestamp, power) tuple
                std::this_thread::sleep_for(std::chrono::milliseconds(impl_->sampling_ms));
                timestamp+=impl_->sampling_ms;
            }
        });
    }

    void PowerProfiler::stop() {
        impl_->running = false;
        if (impl_->worker.joinable()) {
            impl_->worker.join();
        }
        impl_->end_host_energy = impl_->cpu_backend->read_energy(); 
        impl_->end_dev_energy = impl_->gpu_backend->read_energy(); // end energy in uj
       
        /* AMD energy profiling */
        // double dt = static_cast<double>(std::get<0>(power_trace_data[1]) - std::get<0>(power_trace_data[0])) / 1000; // time interval between two consecutive power samples in ms 
        // for (size_t i = 0; i < power_trace_data.size(); ++i){
        //     double p = std::get<1>(power_trace_data[i]);

        //     impl_->total_energy_uj += p * dt; // energy in uj
        // }
        /********************/
    } 

    // data_types::timestamp_t PowerProfiler::get_timestamp() const{
    //     auto now = std::chrono::system_clock::now();
    //     std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    //     std::tm local_time = *std::localtime(&now_c);
    //     auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

    //     std::ostringstream timestamp;
    //     timestamp << std::put_time(&local_time, "%H:%M:%S")
    //                 << ":" << std::setfill('0') << std::setw(3) << ms.count();
    //     return timestamp.str();
    // }

    

    data_types::power_trace_t PowerProfiler::get_power_execution_data() const{
        // Parse the tuple vector in order to remove consecutive power value that are equal.
        // In this way the tuple (t1, p1) and (t2, p2) will have p1 != p2 and in the interval of time [t1, t2] the device operate at power p1.
        data_types::power_trace_t parsed_data;
        std::tuple<data_types::timestamp_t, data_types::power_t> last_power_tuple = power_trace_data.back(); // store the power trace data internally in the power_prof object
      

        std::unique_copy(
            power_trace_data.begin(),
            power_trace_data.end(),
            std::back_inserter(parsed_data),
            [](const auto& a, const auto& b) { // compare prev element with current one
                return std::get<1>(a) == std::get<1>(b);
            }
        );
        parsed_data.push_back(last_power_tuple); // add the last tuple to the parsed data, in order to have the correct end timestamp of the power trace
        return parsed_data;
    }

    /* This funciton can be used when the GPU does not have energy counter but you can only profile the power trace */
    // This function compute the energy consumed by the device according to the power trace.
    // data_types::energy_t  PowerProfiler::compute_energy(const data_types::power_trace_t& trace) const {
    //     if (trace.size() < 2)
    //         return 0;

    //     data_types::energy_t energy = 0;

    //     for (size_t i = 0; i + 1 < trace.size(); ++i)
    //     {
    //         uint64_t p = static_cast<uint64_t>(std::get<1>(trace[i]));

    //         data_types::energy_t dt = impl_->sampling_ms;      // seconds
    //         energy += p * dt;         // Joules
    //     }

    //     return energy;
    // }


    data_types::energy_t PowerProfiler::get_device_energy() const{
        return impl_->end_dev_energy - impl_->start_dev_energy; // uj
        // return impl_->total_energy_uj; // uj
    }

    data_types::energy_t PowerProfiler::get_host_energy() const{
        return impl_->end_host_energy - impl_->start_host_energy; // uj
        // return impl_->total_energy_uj; // uj
    }
}