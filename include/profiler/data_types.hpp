#pragma once
#include <string>
#include <vector>
#include <sstream>
namespace profiler{
    namespace data_types{
        using energy_t = uint64_t;
        using power_t = uint64_t;
        using timestamp_t = std::string;
        using power_trace_t = std::vector<std::tuple<timestamp_t, power_t>>;
        inline std::string power_trace_to_string(const power_trace_t& trace)
        {
            std::ostringstream oss;
            oss << "[";

            for (std::size_t i = 0; i < trace.size(); ++i) {
                const auto& [timestamp, power] = trace[i];
                oss << "(" << timestamp << "," << power << ")";
            }

            oss << "]";
            return oss.str();
        }
    }
}
