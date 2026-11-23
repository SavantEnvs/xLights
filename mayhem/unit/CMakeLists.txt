# Upstream's lean unit suite (tests/unit, XL_TEST_FULL_CORE=OFF), built against the in-image
# copies of the spdlog/pugixml submodules and Catch2 instead of dependencies/ + FetchContent.
cmake_minimum_required(VERSION 3.20)
project(xLights-mayhem-unit CXX C)
get_filename_component(XL_REPO_ROOT "${CMAKE_CURRENT_LIST_DIR}/../.." ABSOLUTE)
set(XL_UNIT "${XL_REPO_ROOT}/tests/unit")
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)
find_package(Catch2 3 REQUIRED CONFIG PATHS "$ENV{XL_CATCH2_DIR}" NO_DEFAULT_PATH)

include(${XL_UNIT}/core_sources.cmake)
list(TRANSFORM XL_TEST_CORE_SOURCES REPLACE "^${XL_REPO_ROOT}/dependencies/pugixml/src" "$ENV{XL_PUGIXML_DIR}/src")
add_library(xlights_core_testlib STATIC ${XL_TEST_CORE_SOURCES})
target_include_directories(xlights_core_testlib PUBLIC
    ${XL_REPO_ROOT}/src-core ${XL_REPO_ROOT}/src-core/utils ${XL_UNIT} ${XL_REPO_ROOT}/include
    $ENV{XL_SPDLOG_DIR}/include $ENV{XL_PUGIXML_DIR}/src)

file(GLOB XL_TEST_SOURCES ${XL_UNIT}/test_*.cpp)
add_executable(xlights_unit_tests ${XL_TEST_SOURCES})
target_link_libraries(xlights_unit_tests PRIVATE xlights_core_testlib Catch2::Catch2WithMain)
set_source_files_properties(${XL_UNIT}/test_float_checks.cpp
    PROPERTIES COMPILE_OPTIONS "-ffast-math;-O2;-Wno-unknown-warning-option;-Wno-nan-infinity-disabled")
