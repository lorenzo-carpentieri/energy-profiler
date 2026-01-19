#include "profiler/power_profiler.hpp"
#include "backend/energy_backend.hpp"
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
        std::unique_ptr<EnergyBackend> backend;
        // PowercapReader host_energy;

        std::atomic<bool> running{false};
        std::thread worker;

        data_types::energy_t start_dev_energy = 0;
        data_types::energy_t end_dev_energy   = 0;

        data_types::energy_t start_host_energy = 0;
        data_types::energy_t end_host_energy   = 0;

        int sampling_ms;
        std::ofstream log;
    };

    PowerProfiler::PowerProfiler(int dev_id,
                                int sampling_rate_ms,
                                const std::string& file)
        : impl_(new Impl) {

        impl_->backend = backend::create_backend();
        impl_->backend->initialize(dev_id);

        impl_->sampling_ms = sampling_rate_ms;
        // Create directory for log file if it doesn't exist
        std::filesystem::path log_dir = std::filesystem::path(file).parent_path();
        if (!std::filesystem::exists(log_dir)) {
            std::filesystem::create_directories(log_dir);
        }

        impl_->log.open(file);
    }

    PowerProfiler::~PowerProfiler() {
        impl_->backend->shutdown(); // shutdown backend
        impl_->log.close(); // close log file
        impl_.reset(); // destroy pointer to backend implementation
    }

    // TODO: Remove power file: timestamp and power will be stored in a vector of tuple 
    void PowerProfiler::start() {
        impl_->running = true;
        impl_->log << "time,power"<<std::endl;

        impl_->start_dev_energy = impl_->backend->read_energy(); // start energy in uj
        // impl_->start_host_energy = impl_->host_energy.read_energy();

        impl_->worker = std::thread([this]() {
            while (impl_->running) {
                std::string timestamp = get_timestamp();
                double power_uw = impl_->backend->read_power(); // read power in uw
                std::tuple<std::string, double> power_tuple = std::make_tuple(timestamp, power_uw);
                power_trace_data.push_back(power_tuple); // Create (timestamp, power) tuple
                impl_->log << timestamp << "," << power_uw * 1e-6 << std::endl; // log timestamp and power in Watts
                std::this_thread::sleep_for(std::chrono::milliseconds(impl_->sampling_ms));
            }
        });
    }

    void PowerProfiler::stop() {
        impl_->running = false;
        if (impl_->worker.joinable()) {
            impl_->worker.join();
        }

        impl_->end_dev_energy = impl_->backend->read_energy(); // end energy in uj
        // // impl_->end_host_energy = impl_->host_energy.read_energy();
    } 

    data_types::timestamp_t PowerProfiler::get_timestamp() const{
        auto now = std::chrono::system_clock::now();
        std::time_t now_c = std::chrono::system_clock::to_time_t(now);
        std::tm local_time = *std::localtime(&now_c);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

        std::ostringstream timestamp;
        timestamp << std::put_time(&local_time, "%H:%M:%S")
                    << ":" << std::setfill('0') << std::setw(3) << ms.count();
        return timestamp.str();
    }

    data_types::energy_t PowerProfiler::get_device_energy() const{
        return impl_->end_dev_energy - impl_->start_dev_energy; // uj
    }

    data_types::power_trace_t PowerProfiler::get_power_execution_data() const{
        // Parse the tuple vector in order to remove consecutive power value that are equal.
        // In this way the tuple (t1, p1) and (t2, p2) will have p1 != p2 and in the interval of time [t1, t2] the device operate at power p1.
        data_types::power_trace_t parsed_data;

        std::unique_copy(
            power_trace_data.begin(),
            power_trace_data.end(),
            std::back_inserter(parsed_data),
            [](const auto& a, const auto& b) { // compare prev element with current one
                return std::get<1>(a) == std::get<1>(b);
            }
        );
        return parsed_data;

    }
}