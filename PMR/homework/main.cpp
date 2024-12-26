#include "allocator.hpp"

#include <benchmark/benchmark.h>
#include <memory_resource>
#include "nlohmann/json.hpp"



std::string load_file(const std::string &path) {
    // std::cerr << std::filesystem::current_path() << std::endl;
    FILE *f = fopen(path.c_str(), "rb");
    fseek(f, 0, SEEK_END);
    auto const size = ftell(f);
    std::string s;
    s.resize(static_cast<std::size_t>(size));
    fseek(f, 0, SEEK_SET);
    auto const nread = fread(&s[0], 1, static_cast<std::size_t>(size), f);
    s.resize(nread);
    fclose(f);
    return s;
}

/*
//  Your homework: get this compiling and working so that PMR permeates the json parser when desired
static void nlohmann_JSON_PMR(benchmark::State &state)
{
    using pmr_json = nlohmann::basic_json<std::map, std::vector, std::pmr::string, bool, std::int64_t, std::uint64_t, double, std::pmr::polymorphic_allocator>;
    auto s = load_file("citm_catalog.json");
    for (auto _ : state) {
        auto jv = pmr_json::parse(s.begin(), s.end());
    }
}
BENCHMARK(nlohmann_JSON_PMR);
*/


struct monotonic {
    using memory_resource_type = std::pmr::monotonic_buffer_resource;
};

template<class T>
using allocator_monotonic = gc::allocator<T, monotonic>;

static void nlohmann_JSON_MBR_stateless_alloc(benchmark::State &state, std::string_view s) {
    constexpr size_t bufsize = 1 << 25;
    thread_local auto buf = std::make_unique_for_overwrite<char[]>(bufsize);
    thread_local std::pmr::monotonic_buffer_resource mbr{buf.get(), bufsize, std::pmr::null_memory_resource()};

    using pmr_json = nlohmann::basic_json<std::map, std::vector, gc::string<monotonic>, bool, std::int64_t,
        std::uint64_t,
        double,
        allocator_monotonic>;
    gc::mr<monotonic> = &mbr;
    for (auto _: state) {
        mbr.release();
        auto jv = pmr_json::parse(s.begin(), s.end());
        benchmark::DoNotOptimize(jv);
    }
}

struct monotonic_devirt {
    using memory_resource_type = gc::monotonic_buffer_resource_devirt;
};

template<class T>
using allocator_monotonic_devirt = gc::allocator<T, monotonic_devirt>;

static void nlohmann_JSON_MBR_stateless_alloc_devirt(benchmark::State &state, std::string_view s) {
    using pmr_json = nlohmann::basic_json<std::map, std::vector, gc::string<monotonic_devirt>, bool, std::int64_t,
        std::uint64_t, double,
        allocator_monotonic_devirt>;
    constexpr size_t bufsize = 1 << 25;
    thread_local auto buf = std::make_unique_for_overwrite<char[]>(bufsize);

    thread_local gc::monotonic_buffer_resource_devirt mbr{buf.get(), bufsize, std::pmr::null_memory_resource()};
    gc::mr<monotonic_devirt> = &mbr;
    for (auto _: state) {
        mbr.release();
        auto jv = pmr_json::parse(s.begin(), s.end());
        benchmark::DoNotOptimize(jv);
    }
}

static void nlohmann_JSON_Default(benchmark::State &state, std::string_view s) {
    for (auto _: state) {
        auto jv = nlohmann::json::parse(s.begin(), s.end());
        benchmark::DoNotOptimize(jv);
    }
}

static void JSON_Perf(benchmark::State &state, void (*test)(benchmark::State &, const std::string_view),
                      const std::string &filename) {
    auto s = load_file(filename);
    test(state, s);
    state.SetBytesProcessed(static_cast<long long int>(state.iterations() * s.size()));
}

#define ADD_BENCHMARK(func, filename) BENCHMARK_CAPTURE(JSON_Perf, func-filename, func, filename)

ADD_BENCHMARK(nlohmann_JSON_Default, "citm_catalog.json");
ADD_BENCHMARK(nlohmann_JSON_MBR_stateless_alloc, "citm_catalog.json");
ADD_BENCHMARK(nlohmann_JSON_MBR_stateless_alloc_devirt, "citm_catalog.json");

ADD_BENCHMARK(nlohmann_JSON_Default, "gsoc-2018.json");
ADD_BENCHMARK(nlohmann_JSON_MBR_stateless_alloc, "gsoc-2018.json");
ADD_BENCHMARK(nlohmann_JSON_MBR_stateless_alloc_devirt, "gsoc-2018.json");

ADD_BENCHMARK(nlohmann_JSON_Default, "github_events.json");
ADD_BENCHMARK(nlohmann_JSON_MBR_stateless_alloc, "github_events.json");
ADD_BENCHMARK(nlohmann_JSON_MBR_stateless_alloc_devirt, "github_events.json");


BENCHMARK_MAIN();
