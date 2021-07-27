cmake_minimum_required(VERSION 3.11)
project(ppl.common)

include(cmake/deps.cmake)

# --------------------------------------------------------------------------- #

option(PPLCOMMON_BUILD_TESTS "build tests" ON)
option(PPLCOMMON_BUILD_BENCHMARK "build benchmark" ON)
option(PPLCOMMON_ENABLE_PYTHON_API "enable python api support" ON)

# --------------------------------------------------------------------------- #

if(PPLCOMMON_ENABLE_PYTHON_API)
    add_subdirectory(python)
endif()

file(GLOB PPLCOMMON_SRC
    src/ppl/common/*.cc)

set(PPLCOMMON_INCLUDES)
list(APPEND PPLCOMMON_INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}/src)

set(PPLCOMMON_LINK_LIBRARIES)
set(PPLCOMMON_DEFINITIONS)

include(cmake/x86.cmake)

list(FILTER PPLCOMMON_SRC EXCLUDE REGEX "(.*)_unittest\\.cc$")
list(FILTER PPLCOMMON_SRC EXCLUDE REGEX "(.*)_benchmark\\.cc$")

# --------------------------------------------------------------------------- #

add_library(pplcommon_static STATIC ${PPLCOMMON_SRC})
target_link_libraries(pplcommon_static PUBLIC ${PPLCOMMON_LINK_LIBRARIES})
target_include_directories(pplcommon_static PUBLIC ${PPLCOMMON_INCLUDES})
target_compile_definitions(pplcommon_static PUBLIC ${PPLCOMMON_DEFINITIONS})

include(cmake/install.cmake)

if(PPLCOMMON_BUILD_TESTS)
    include(cmake/unittest.cmake)
endif()

if(PPLCOMMON_BUILD_BENCHMARK)
    include(cmake/benchmark.cmake)
endif()
