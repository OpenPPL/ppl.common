if(NOT HPCC_DEPS_DIR)
    set(HPCC_DEPS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/deps)
endif()

include(FetchContent)

set(FETCHCONTENT_BASE_DIR ${HPCC_DEPS_DIR})
set(FETCHCONTENT_QUIET OFF)

if(PPLCOMMON_HOLD_DEPS)
    set(FETCHCONTENT_UPDATES_DISCONNECTED ON)
endif()

# --------------------------------------------------------------------------- #

set(__HPCC_COMMIT__ af7dcc6c1b1eaf622b3d01472b89ce62d881f66c)

if(PPLCOMMON_DEP_HPCC_PKG)
    FetchContent_Declare(hpcc
        URL ${PPLCOMMON_DEP_HPCC_PKG}
        SOURCE_DIR ${HPCC_DEPS_DIR}/hpcc
        BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/hpcc-build
        SUBBUILD_DIR ${HPCC_DEPS_DIR}/hpcc-subbuild)
else()
    if(NOT PPLCOMMON_DEP_HPCC_GIT)
        set(PPLCOMMON_DEP_HPCC_GIT "https://github.com/openppl-public/hpcc.git")
    endif()
    FetchContent_Declare(hpcc
        GIT_REPOSITORY ${PPLCOMMON_DEP_HPCC_GIT}
        GIT_TAG ${__HPCC_COMMIT__}
        SOURCE_DIR ${HPCC_DEPS_DIR}/hpcc
        BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/hpcc-build
        SUBBUILD_DIR ${HPCC_DEPS_DIR}/hpcc-subbuild)
endif()

unset(__HPCC_COMMIT__)

FetchContent_GetProperties(hpcc)
if(NOT hpcc_POPULATED)
    FetchContent_Populate(hpcc)
    include(${hpcc_SOURCE_DIR}/cmake/hpcc-common.cmake)
endif()

# --------------------------------------------------------------------------- #

set(PYBIND11_INSTALL OFF CACHE BOOL "disable pybind11 installation")
set(PYBIND11_TEST OFF CACHE BOOL "disable pybind11 tests")
set(PYBIND11_NOPYTHON ON CACHE BOOL "do not find python")
set(PYBIND11_FINDPYTHON OFF CACHE BOOL "do not find python")

set(__PYBIND11_TAG__ v2.9.2)

if(PPLCOMMON_DEP_PYBIND11_PKG)
    hpcc_declare_pkg_dep(pybind11
        ${PPLCOMMON_DEP_PYBIND11_PKG})
else()
    if(NOT PPLCOMMON_DEP_PYBIND11_GIT)
        set(PPLCOMMON_DEP_PYBIND11_GIT "https://github.com/pybind/pybind11.git")
    endif()
    hpcc_declare_git_dep(pybind11
        ${PPLCOMMON_DEP_PYBIND11_GIT}
        ${__PYBIND11_TAG__})
endif()

unset(__PYBIND11_TAG__)

# --------------------------------------------------------------------------- #

set(LUACPP_INSTALL OFF CACHE BOOL "")
set(LUACPP_BUILD_TESTS OFF CACHE BOOL "")

set(__LUACPP_COMMIT__ d4e60a321a19a05a34bd15d3d508647f394007f3)

if(PPLCOMMON_DEP_LUACPP_PKG)
    hpcc_declare_pkg_dep(luacpp
        ${PPLCOMMON_DEP_LUACPP_PKG})
else()
    if(NOT PPLCOMMON_DEP_LUACPP_GIT)
        set(PPLCOMMON_DEP_LUACPP_GIT "https://github.com/ouonline/luacpp.git")
    endif()
    hpcc_declare_git_dep(luacpp
        ${PPLCOMMON_DEP_LUACPP_GIT}
        ${__LUACPP_COMMIT__})
endif()

unset(__LUACPP_COMMIT__)

# --------------------------------------------------------------------------- #

set(BUILD_GMOCK OFF CACHE BOOL "")
Set(INSTALL_GTEST OFF CACHE BOOL "")

set(__GOOGLETEST_TAG__ release-1.10.0)

if(PPLCOMMON_DEP_GOOGLETEST_PKG)
    hpcc_declare_pkg_dep(googletest
        ${PPLCOMMON_DEP_GOOGLETEST_PKG})
else()
    if(NOT PPLCOMMON_DEP_GOOGLETEST_GIT)
        set(PPLCOMMON_DEP_GOOGLETEST_GIT "https://github.com/google/googletest.git")
    endif()
    hpcc_declare_git_dep(googletest
        ${PPLCOMMON_DEP_GOOGLETEST_GIT}
        ${__GOOGLETEST_TAG__})
endif()

unset(__GOOGLETEST_TAG__)

# --------------------------------------------------------------------------- #

set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "disable benchmark tests")
set(BENCHMARK_ENABLE_INSTALL OFF CACHE BOOL "")

set(__BENCHMARK__TAG__ v1.5.6)

if(PPLCOMMON_DEP_BENCHMARK_PKG)
    hpcc_declare_pkg_dep(benchmark
        ${PPLCOMMON_DEP_BENCHMARK_PKG})
else()
    if(NOT PPLCOMMON_DEP_BENCHMARK_GIT)
        set(PPLCOMMON_DEP_BENCHMARK_GIT "https://github.com/google/benchmark.git")
    endif()
    hpcc_declare_git_dep(benchmark
        ${PPLCOMMON_DEP_BENCHMARK_GIT}
        ${__BENCHMARK__TAG__})
endif()

unset(__BENCHMARK__TAG__)
