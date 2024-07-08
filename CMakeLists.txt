cmake_minimum_required(VERSION 3.14)
project(ppl.common)

include(cmake/deps.cmake)

# --------------------------------------------------------------------------- #

option(PPLCOMMON_BUILD_TESTS "build tests" ON)
option(PPLCOMMON_BUILD_BENCHMARK "build benchmark" ON)
option(PPLCOMMON_INSTALL "install ppl.common headers and libs" ON)
option(PPLCOMMON_ENABLE_PYTHON_API "enable python api support" OFF)
option(PPLCOMMON_HOLD_DEPS "don't update dependencies" OFF)
option(PPLCOMMON_STDIO_LOG_COLOR "" ON)

option(PPLCOMMON_USE_X86_64 "" OFF)
option(PPLCOMMON_USE_AARCH64 "" OFF)
option(PPLCOMMON_USE_ARMV7 "" OFF)
option(PPLCOMMON_USE_CUDA "" OFF)
option(PPLCOMMON_USE_OPENCL "" OFF)

# deprecated options
if(HPCC_USE_X86_64)
    message(FATAL_ERROR "`HPCC_USE_X86_64` is deprecated. use `PPLCOMMON_USE_X86_64` instead.")
endif()
if(HPCC_USE_AARCH64)
    message(FATAL_ERROR "`HPCC_USE_AARCH64` is deprecated. use `PPLCOMMON_USE_AARCH64` instead.")
endif()
if(HPCC_USE_CUDA)
    message(FATAL_ERROR "`HPCC_USE_CUDA` is deprecated. use `PPLCOMMON_USE_CUDA` instead.")
endif()

# --------------------------------------------------------------------------- #

if(PPLCOMMON_ENABLE_PYTHON_API)
    add_subdirectory(python)
endif()

file(GLOB PPLCOMMON_SRC
    src/ppl/common/*.cc)

if(MSVC)
    file(GLOB __win_src__ src/ppl/common/windows/*.cc)
    list(APPEND PPLCOMMON_SRC ${__win_src__})
    unset(__win_src__)
endif()

set(PPLCOMMON_INCLUDES)
list(APPEND PPLCOMMON_INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}/src)

set(PPLCOMMON_LINK_LIBRARIES)
set(PPLCOMMON_LINK_DIRECTORIES)
set(PPLCOMMON_DEFINITIONS)

if(PPLCOMMON_USE_X86_64)
    include(cmake/x86.cmake)
endif()

if(PPLCOMMON_USE_AARCH64 OR PPLCOMMON_USE_ARMV7)
    include(cmake/arm.cmake)
endif()

if(PPLCOMMON_USE_OPENCL)
    include(cmake/ocl.cmake)
endif()

if(PPLCOMMON_USE_CUDA)
    include(cmake/cuda.cmake)
endif()

if(PPLCOMMON_STDIO_LOG_COLOR)
    list(APPEND PPLCOMMON_DEFINITIONS PPLCOMMON_STDIO_LOG_COLOR)
endif()

list(FILTER PPLCOMMON_SRC EXCLUDE REGEX "(.*)_unittest\\.cc$")
list(FILTER PPLCOMMON_SRC EXCLUDE REGEX "(.*)_benchmark\\.cc$")

# --------------------------------------------------------------------------- #

# for pplcommon_static and pplcommon-config.cmake
if(NOT MSVC)
    if(NOT ${CMAKE_SYSTEM_NAME} STREQUAL "Android" AND NOT ANDROID)
        list(APPEND PPLCOMMON_LINK_LIBRARIES pthread)
    endif()
endif()

add_library(pplcommon_static STATIC ${PPLCOMMON_SRC})
target_link_libraries(pplcommon_static PUBLIC ${PPLCOMMON_LINK_LIBRARIES})
target_link_directories(pplcommon_static PUBLIC ${PPLCOMMON_LINK_DIRECTORIES})
target_include_directories(pplcommon_static PUBLIC ${PPLCOMMON_INCLUDES})
target_compile_definitions(pplcommon_static PUBLIC ${PPLCOMMON_DEFINITIONS})
target_compile_features(pplcommon_static PUBLIC cxx_std_11)

if(PPLCOMMON_INSTALL)
    include(cmake/install.cmake)
endif()

if(PPLCOMMON_BUILD_TESTS)
    include(cmake/unittest.cmake)
endif()

if(PPLCOMMON_BUILD_BENCHMARK)
    include(cmake/benchmark.cmake)
endif()
