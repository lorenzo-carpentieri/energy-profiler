#pragma once
#include <string>
#include <cstdint>
#include <memory>

class PowerProfiler {
public:
    PowerProfiler(int device_id,
                  int sampling_rate_ms,
                  const std::string& output_file);

    ~PowerProfiler();

    void start();
    void stop();

    // double device_energy_joules() const;
    double get_device_energy() const; // energy consumed by the device with device_id in joules

private:
    struct Impl;                 // opaque PIMPL
    std::unique_ptr<Impl> impl_;                 // std::unique_ptr<Impl>
    std::string get_timestamp() const;
};
