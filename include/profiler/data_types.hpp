#include <string>
#include <vector>
namespace profiler{
    namespace data_types{
        using energy_t = uint64_t;
        using power_t = uint64_t;
        using timestamp_t = std::string;
        using power_trace_t = std::vector<std::tuple<timestamp_t, power_t>>;
    }
}
