add_executable(pmr_performance_tests performance_tests.cpp)
target_link_libraries(
  pmr_performance_tests PRIVATE CONAN_PKG::benchmark project_options
                                project_warnings)

include(FetchContent)
FetchContent_Declare(rapidjson
                     GIT_REPOSITORY https://github.com/Tencent/rapidjson.git)
FetchContent_Declare(nlohmann
                     GIT_REPOSITORY https://github.com/nlohmann/json.git)
FetchContent_Declare(boost_json
                     GIT_REPOSITORY https://github.com/boostorg/json.git)

FetchContent_Populate(rapidjson)
FetchContent_Populate(nlohmann)
FetchContent_Populate(boost_json)

add_executable(json_pmr_performance_tests json_tests.cpp ${boost_json_SOURCE_DIR}/src/src.cpp)
target_link_libraries(
  json_pmr_performance_tests PRIVATE CONAN_PKG::benchmark project_options
                                     project_warnings)

target_compile_definitions(json_pmr_performance_tests
                           PRIVATE "BOOST_JSON_STANDALONE")
target_include_directories(
  json_pmr_performance_tests SYSTEM
  PRIVATE ${boost_json_SOURCE_DIR}/include ${boost_json_SOURCE_DIR}/test
          ${nlohmann_SOURCE_DIR}/single_include ${rapidjson_SOURCE_DIR}/include)
