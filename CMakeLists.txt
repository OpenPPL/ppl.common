cmake_minimum_required(VERSION 3.14)
project(ppl.common)

include(cmake/deps.cmake)

# --------------------------------------------------------------------------- #

option(PPLCOMMON_BUILD_TESTS "build tests" ON)
option(PPLCOMMON_BUILD_BENCHMARK "build benchmark" ON)
option(PPLCOMMON_INSTALL "install ppl.common headers and libs" ON)
option(PPLCOMMON_ENABLE_PYTHON_API "enable python api support" OFF)
option(PPLCOMMON_ENABLE_LUA_API "enable lua api support" OFF)
option(PPLCOMMON_HOLD_DEPS "don't update dependencies" OFF)

# --------------------------------------------------------------------------- #

if(PPLCOMMON_ENABLE_PYTHON_API)
    add_subdirectory(python)
endif()

if(PPLCOMMON_ENABLE_LUA_API)
    add_subdirectory(lua)
endif()

file(GLOB PPLCOMMON_SRC
    src/ppl/common/*.cc)

set(PPLCOMMON_INCLUDES)
list(APPEND PPLCOMMON_INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}/src)

set(PPLCOMMON_LINK_LIBRARIES)
set(PPLCOMMON_DEFINITIONS)

if(HPCC_USE_X86_64)
    include(cmake/x86.cmake)
endif()

if(HPCC_USE_AARCH64)
    include(cmake/arm.cmake)
endif()

list(FILTER PPLCOMMON_SRC EXCLUDE REGEX "(.*)_unittest\\.cc$")
list(FILTER PPLCOMMON_SRC EXCLUDE REGEX "(.*)_benchmark\\.cc$")

# --------------------------------------------------------------------------- #

add_library(pplcommon_static STATIC ${PPLCOMMON_SRC})
target_link_libraries(pplcommon_static PUBLIC ${PPLCOMMON_LINK_LIBRARIES})
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
