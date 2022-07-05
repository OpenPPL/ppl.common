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

set(__HPCC_COMMIT__ 614c481aa9b8ff949ddfba70a3ae1aefa39ffd58)

if(PPLCOMMON_DEP_HPCC_PKG)
    FetchContent_Declare(hpcc
        URL ${PPLCOMMON_DEP_HPCC_PKG}
        SOURCE_DIR ${HPCC_DEPS_DIR}/hpcc
        BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/hpcc-build
        SUBBUILD_DIR ${HPCC_DEPS_DIR}/hpcc-subbuild)
elseif(PPLCOMMON_DEP_HPCC_GIT)
    FetchContent_Declare(hpcc
        GIT_REPOSITORY ${PPLCOMMON_DEP_HPCC_GIT}
        GIT_TAG ${__HPCC_COMMIT__}
        SOURCE_DIR ${HPCC_DEPS_DIR}/hpcc
        BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/hpcc-build
        SUBBUILD_DIR ${HPCC_DEPS_DIR}/hpcc-subbuild)
else()
    FetchContent_Declare(hpcc
        URL https://github.com/openppl-public/hpcc/archive/${__HPCC_COMMIT__}.zip
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
elseif(PPLCOMMON_DEP_PYBIND11_GIT)
    hpcc_declare_git_dep(pybind11
        ${PPLCOMMON_DEP_PYBIND11_GIT}
        ${__PYBIND11_TAG__})
else()
    hpcc_declare_pkg_dep(pybind11
        https://github.com/pybind/pybind11/archive/refs/tags/${__PYBIND11_TAG__}.zip)
endif()

unset(__PYBIND11_TAG__)

# --------------------------------------------------------------------------- #

set(LUACPP_INSTALL OFF CACHE BOOL "")
set(LUACPP_BUILD_TESTS OFF CACHE BOOL "")

set(__LUACPP_COMMIT__ f381a4702017b61ee9662ae9fa7bceec8b5c7b32)

if(PPLCOMMON_DEP_LUACPP_PKG)
    hpcc_declare_pkg_dep(luacpp
        ${PPLCOMMON_DEP_LUACPP_PKG})
elseif(PPLCOMMON_DEP_LUACPP_GIT)
    hpcc_declare_git_dep(luacpp
        ${PPLCOMMON_DEP_LUACPP_GIT}
        ${__LUACPP_COMMIT__})
else()
    hpcc_declare_pkg_dep(luacpp
        https://github.com/ouonline/lua-cpp/archive/${__LUACPP_COMMIT__}.zip)
endif()

unset(__LUACPP_COMMIT__)

# --------------------------------------------------------------------------- #

set(BUILD_GMOCK OFF CACHE BOOL "")
Set(INSTALL_GTEST OFF CACHE BOOL "")

set(__GOOGLETEST_TAG__ release-1.10.0)

if(PPLCOMMON_DEP_GOOGLETEST_PKG)
    hpcc_declare_pkg_dep(googletest
        ${PPLCOMMON_DEP_GOOGLETEST_PKG})
elseif(PPLCOMMON_DEP_GOOGLETEST_GIT)
    hpcc_declare_git_dep(googletest
        ${PPLCOMMON_DEP_GOOGLETEST_GIT}
        ${__GOOGLETEST_TAG__})
else()
    hpcc_declare_pkg_dep(googletest
        https://github.com/google/googletest/archive/refs/tags/${__GOOGLETEST_TAG__}.zip)
endif()

unset(__GOOGLETEST_TAG__)

# --------------------------------------------------------------------------- #

set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "disable benchmark tests")
set(BENCHMARK_ENABLE_INSTALL OFF CACHE BOOL "")

set(__BENCHMARK__TAG__ v1.5.6)

if(PPLCOMMON_DEP_BENCHMARK_PKG)
    hpcc_declare_pkg_dep(benchmark
        ${PPLCOMMON_DEP_BENCHMARK_PKG})
elseif(PPLCOMMON_DEP_BENCHMARK_GIT)
    hpcc_declare_git_dep(benchmark
        ${PPLCOMMON_DEP_BENCHMARK_GIT}
        ${__BENCHMARK__TAG__})
else()
    hpcc_declare_pkg_dep(benchmark
        https://github.com/google/benchmark/archive/refs/tags/${__BENCHMARK__TAG__}.zip)
endif()

unset(__BENCHMARK__TAG__)
