cmake_minimum_required(VERSION 3.29)
project(pmr_homework_guoci)

message("Building with CMake version: ${CMAKE_VERSION}")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(bm main.cpp)

include(FetchContent)
FetchContent_Declare(googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG main)
FetchContent_MakeAvailable(googletest)

FetchContent_Declare(benchmark
        GIT_REPOSITORY https://github.com/google/benchmark.git
        GIT_TAG main)
FetchContent_MakeAvailable(benchmark)

FetchContent_Declare(nlohmann_json
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG master)
FetchContent_MakeAvailable(nlohmann_json)

find_package(benchmark REQUIRED)

target_link_libraries(bm PRIVATE benchmark::benchmark nlohmann_json::nlohmann_json)

target_compile_options(bm PRIVATE
        -Wall -Wextra -Wshadow -Wconversion -pedantic -Wstrict-overflow
        -Wnrvo
        -Wuninitialized
)