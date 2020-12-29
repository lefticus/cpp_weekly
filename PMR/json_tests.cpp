//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/json
//

#include <boost/json/detail/config.hpp>

#if defined(BOOST_JSON_USE_SSE2)
#define RAPIDJSON_SSE2
#define SSE2_ARCH_SUFFIX "/sse2"
#else
#define SSE2_ARCH_SUFFIX ""
#endif

#include <benchmark/benchmark.h>
#include "nlohmann/json.hpp"

#include "rapidjson/document.h"
#include "rapidjson/rapidjson.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"

#include <algorithm>
#include <boost/json.hpp>
#include <boost/json/basic_parser_impl.hpp>
#include <chrono>
#include <cstdio>
#include <iostream>
#include <memory>
#include <vector>
#include <memory_resource>
#include "test_suite.hpp"

/*  References

    https://github.com/nst/JSONTestSuite

    http://seriot.ch/parsing_json.php
*/


::test_suite::debug_stream dout(std::cerr);
std::stringstream strout;

std::string load_file(const std::string &path)
{
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


static void Boost_JSON_Default_Parse(benchmark::State &state, std::string_view s)
{
  for (auto _ : state) {
    boost::json::stream_parser p;
    boost::json::error_code ec;
    p.write(s.data(), s.size(), ec);
    if (!ec) {
      p.finish(ec);
    }
    if (!ec) {
      auto jv = p.release();
    }
  }
}

static void Boost_JSON_PMR_Monotonic_Winkout_Parse(benchmark::State &state, std::string_view s)
{
  for (auto _ : state) {
    std::pmr::monotonic_buffer_resource mr;
    std::pmr::polymorphic_allocator<> pa{ &mr };
    auto &p = *pa.new_object<boost::json::stream_parser>();
    p.reset(pa);
    boost::json::error_code ec;

    p.write(s.data(), s.size(), ec);
    if (!ec)
      p.finish(ec);
    // all data is freed when the buffer resource goes away, or at least should be
    //      if (!ec)
    //        auto jv = p.release();
  }
}

//  Your homework: get this compiling and working so that PMR permeates the json parser when desired
//  static void nlohmann_JSON_PMR(benchmark::State &state)
//  {
//    using pmr_json = nlohmann::basic_json<std::map, std::vector, std::pmr::string, bool, std::int64_t, std::uint64_t, double, std::pmr::polymorphic_allocator>;
//    auto s = load_file("citm_catalog.json");
//    for (auto _ : state) {
//        auto jv = pmr_json::parse(s.begin(), s.end());
//    }
//  }
//  BENCHMARK(nlohmann_JSON_PMR);

static void Boost_JSON_PMR_Monotonic_Parse(benchmark::State &state, std::string_view s)
{
  for (auto _ : state) {
    boost::json::stream_parser p;
    std::pmr::monotonic_buffer_resource mr;
    std::pmr::polymorphic_allocator<> pa{ &mr };

    boost::json::error_code ec;
    p.reset(pa); // it seems this reset is necessary, if you pass it to the
                 // constructor for stream_parser it doesn't work as expected
    p.write(s.data(), s.size(), ec);
    if (!ec)
      p.finish(ec);
    if (!ec)
      auto jv = p.release();
  }
}

static void Boost_JSON_PMR_Pool_Monotonic_Parse(benchmark::State &state, std::string_view s)
{
  // because the pool resource frees memory when asked, we can move it out of the loop
  std::pmr::monotonic_buffer_resource upstream{1000000};
  std::pmr::unsynchronized_pool_resource mr{&upstream};
  std::pmr::polymorphic_allocator<> pa{ &mr };

  for (auto _ : state) {
    boost::json::stream_parser p;
    boost::json::error_code ec;
    p.reset(pa); // it seems this reset is necessary, if you pass it to the
                 // constructor for stream_parser it doesn't work as expected
    p.write(s.data(), s.size(), ec);
    if (!ec)
      p.finish(ec);
    if (!ec)
      auto jv = p.release();
  }
}

struct RapidJSONPMRAlloc
{
  std::pmr::memory_resource *upstream = std::pmr::get_default_resource();

  static constexpr bool kNeedFree = true;

  static constexpr auto objectOffset = alignof(std::max_align_t);
  static constexpr auto memPadding = objectOffset * 2;

  void *Malloc(size_t size)
  {
    if (size != 0) {
      const auto allocated_size = size + memPadding;
      std::byte *newPtr = static_cast<std::byte *>(upstream->allocate(allocated_size));
      auto *ptrToReturn = newPtr + memPadding;
      // placement new a pointer to ourselves at the first memory location
      new (newPtr)(RapidJSONPMRAlloc *)(this);
      // placement new the size in the second location
      new (newPtr + objectOffset)(std::size_t)(size);
      return ptrToReturn;
    } else {
      return nullptr;
    }
  }

  void freePtr(void *origPtr, size_t originalSize)
  {
    if (origPtr == nullptr) {
      return;
    }
    upstream->deallocate(static_cast<std::byte *>(origPtr) - memPadding, originalSize + memPadding);
  }

  void *Realloc(void *origPtr, size_t originalSize, size_t newSize)
  {
    if (newSize == 0) {
      freePtr(origPtr, originalSize);
      return nullptr;
    }

    if (newSize <= originalSize) {
      return origPtr;
    }

    void *newPtr = Malloc(newSize);
    std::memcpy(newPtr, origPtr, originalSize);
    freePtr(origPtr, originalSize);
    return newPtr;
  }

  // and Free needs to be static, which causes this whole thing
  // to fall apart. This means that we have to keep our own list of allocated memory
  // with our own pointers back to ourselves and our own list of sizes
  // so we can push all of this back to the upstream allocator
  static void Free(void *ptr)
  {
    if (ptr == nullptr ) {
      return;
    }

    std::byte *startOfData = static_cast<std::byte *>(ptr) - memPadding;

    auto *ptrToAllocator = *reinterpret_cast<RapidJSONPMRAlloc **>(startOfData);
    auto origAllocatedSize = *reinterpret_cast<std::size_t *>(startOfData + objectOffset);

    ptrToAllocator->freePtr(ptr, origAllocatedSize);
  }
};


static void RapidJSON_PMR_Parse(benchmark::State &state, std::string_view s)
{
  for (auto _ : state) {
    using namespace rapidjson;
    RapidJSONPMRAlloc alloc;
    GenericDocument<UTF8<>, RapidJSONPMRAlloc> d(&alloc);
    d.Parse(s.data(), s.size());
  }
}

static void RapidJSON_PMR_Monotonic_Parse(benchmark::State &state, std::string_view s)
{
  for (auto _ : state) {
    using namespace rapidjson;
    std::pmr::monotonic_buffer_resource mr;
    RapidJSONPMRAlloc alloc{&mr};
    GenericDocument<UTF8<>, RapidJSONPMRAlloc> d(&alloc);
    d.Parse(s.data(), s.size());
  }
}

static void RapidJSON_PMR_Pool_Monotonic_Parse(benchmark::State &state, std::string_view s)
{
  for (auto _ : state) {
    using namespace rapidjson;
    std::pmr::monotonic_buffer_resource upstream{1000000};
    std::pmr::unsynchronized_pool_resource mr{&upstream};
    RapidJSONPMRAlloc alloc{&mr};
    GenericDocument<UTF8<>, RapidJSONPMRAlloc> d(&alloc);
    d.Parse(s.data(), s.size());
  }
}


static void RapidJSON_PMR_Monotonic_Winkout_Parse(benchmark::State &state, std::string_view s)
{
  for (auto _ : state) {
    using namespace rapidjson;
    std::pmr::monotonic_buffer_resource mr;
    RapidJSONPMRAlloc alloc{&mr};
    std::pmr::polymorphic_allocator<> pa{ &mr };
    auto &d = *pa.new_object<GenericDocument<UTF8<>, RapidJSONPMRAlloc>>(&alloc);
    d.Parse(s.data(), s.size());
  }
}


static void RapidJSON_Monotonic_Parse(benchmark::State &state, std::string_view s)
{
  for (auto _ : state) {
    using namespace rapidjson;
    std::string monotonic_buffer{s};
    rapidjson::Document d;
    d.ParseInsitu(monotonic_buffer.data());
  }
}

static void RapidJSON_CRT_Parse(benchmark::State &state, std::string_view s)
{
  for (auto _ : state) {
    using namespace rapidjson;
    CrtAllocator alloc;
    GenericDocument<UTF8<>, CrtAllocator> d(&alloc);
    d.Parse(s.data(), s.size());
  }
}

static void RapidJSON_Default_Parse(benchmark::State &state, std::string_view s)
{
  for (auto _ : state) {
    rapidjson::Document d;
    d.Parse(s.data(), s.size());
  }
}

static void nlohmann_JSON_Default(benchmark::State &state, std::string_view s)
{
  for (auto _ : state) {
    auto jv = nlohmann::json::parse(s.begin(), s.end());
  }
}

static void JSON_Perf(benchmark::State &state, void (*test)(benchmark::State &, const std::string_view), const std::string &filename) {
  auto s = load_file(filename);
  test(state, s);
  state.SetBytesProcessed(static_cast<long long int>(state.iterations() * s.size()));
}

#define ADD_BENCHMARK(func, filename) BENCHMARK_CAPTURE(JSON_Perf, func-filename, func, filename)
ADD_BENCHMARK(Boost_JSON_Default_Parse, "citm_catalog.json");
ADD_BENCHMARK(Boost_JSON_PMR_Monotonic_Winkout_Parse, "citm_catalog.json");
ADD_BENCHMARK(Boost_JSON_PMR_Monotonic_Parse, "citm_catalog.json");
ADD_BENCHMARK(Boost_JSON_PMR_Pool_Monotonic_Parse, "citm_catalog.json");
ADD_BENCHMARK(RapidJSON_Default_Parse, "citm_catalog.json");
ADD_BENCHMARK(RapidJSON_CRT_Parse, "citm_catalog.json");
ADD_BENCHMARK(RapidJSON_PMR_Parse, "citm_catalog.json");
ADD_BENCHMARK(RapidJSON_PMR_Monotonic_Parse, "citm_catalog.json");
ADD_BENCHMARK(RapidJSON_PMR_Pool_Monotonic_Parse, "citm_catalog.json");
ADD_BENCHMARK(RapidJSON_PMR_Monotonic_Winkout_Parse, "citm_catalog.json");
ADD_BENCHMARK(RapidJSON_Monotonic_Parse, "citm_catalog.json");
ADD_BENCHMARK(nlohmann_JSON_Default, "citm_catalog.json");

ADD_BENCHMARK(Boost_JSON_Default_Parse, "gsoc-2018.json");
ADD_BENCHMARK(Boost_JSON_PMR_Monotonic_Winkout_Parse, "gsoc-2018.json");
ADD_BENCHMARK(Boost_JSON_PMR_Monotonic_Parse, "gsoc-2018.json");
ADD_BENCHMARK(RapidJSON_Default_Parse, "gsoc-2018.json");
ADD_BENCHMARK(RapidJSON_CRT_Parse, "gsoc-2018.json");
ADD_BENCHMARK(RapidJSON_PMR_Parse, "gsoc-2018.json");
ADD_BENCHMARK(RapidJSON_PMR_Monotonic_Parse, "gsoc-2018.json");
ADD_BENCHMARK(RapidJSON_PMR_Pool_Monotonic_Parse, "gsoc-2018.json");
ADD_BENCHMARK(RapidJSON_PMR_Monotonic_Winkout_Parse, "gsoc-2018.json");
ADD_BENCHMARK(RapidJSON_Monotonic_Parse, "gsoc-2018.json");
ADD_BENCHMARK(nlohmann_JSON_Default, "gsoc-2018.json");

ADD_BENCHMARK(Boost_JSON_Default_Parse, "github_events.json");
ADD_BENCHMARK(Boost_JSON_PMR_Monotonic_Winkout_Parse, "github_events.json");
ADD_BENCHMARK(Boost_JSON_PMR_Monotonic_Parse, "github_events.json");
ADD_BENCHMARK(RapidJSON_Default_Parse, "github_events.json");
ADD_BENCHMARK(RapidJSON_CRT_Parse, "github_events.json");
ADD_BENCHMARK(RapidJSON_PMR_Parse, "github_events.json");
ADD_BENCHMARK(RapidJSON_PMR_Monotonic_Parse, "github_events.json");
ADD_BENCHMARK(RapidJSON_PMR_Pool_Monotonic_Parse, "github_events.json");
ADD_BENCHMARK(RapidJSON_PMR_Monotonic_Winkout_Parse, "github_events.json");
ADD_BENCHMARK(RapidJSON_Monotonic_Parse, "github_events.json");
ADD_BENCHMARK(nlohmann_JSON_Default, "github_events.json");







