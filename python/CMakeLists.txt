hpcc_populate_dep(pybind11)

set(__python_soabi__ )

if(NOT PYTHON3_INCLUDE_DIRS)
    find_package(Python3 COMPONENTS Development QUIET)
    if(NOT Python3_Development_FOUND)
        if(MSVC)
            message(FATAL_ERROR "python dev lib is required. please install python3 development files or specify `PYTHON3_INCLUDE_DIRS` and `PYTHON3_LIBRARIES` manually.")
        else()
            message(FATAL_ERROR "python dev lib is required. please install python3 development files (python3-dev on Debian/Ubuntu or python3-devel on RetHat/CentOS) or specify `PYTHON3_INCLUDE_DIRS` manually.")
        endif()
    endif()

    # `Python3_SOABI` is provided by find_package(Python3) in cmake >= 3.17
    set(__python_soabi__ ${Python3_SOABI})

    set(PYTHON3_INCLUDE_DIRS ${Python3_INCLUDE_DIRS})
    if(MSVC) # cannot link static libraries built without -fPIC on linux
        set(PYTHON3_LIBRARIES ${Python3_LIBRARIES})
    endif()
endif()

if(MSVC)
    if(NOT PYTHON3_LIBRARIES)
        message(FATAL_ERROR "please specify `PYTHON3_INCLUDE_DIRS` and `PYTHON3_LIBRARIES` manually.")
    endif()
endif()

file(GLOB_RECURSE PPLCOMMON_PYTHON_API_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cc)
add_library(pypplcommon_shared SHARED ${PPLCOMMON_PYTHON_API_SRC})
target_link_libraries(pypplcommon_shared PUBLIC pplcommon_static ${PYTHON3_LIBRARIES})
target_include_directories(pypplcommon_shared PUBLIC
    ${pybind11_SOURCE_DIR}/include
    ${PYTHON3_INCLUDE_DIRS})
target_compile_features(pypplcommon_shared PUBLIC cxx_std_11)

# --------------------------------------------------------------------------- #

# rename target for python import
set_target_properties(pypplcommon_shared PROPERTIES PREFIX "")
set_target_properties(pypplcommon_shared PROPERTIES OUTPUT_NAME "common")

if(__python_soabi__)
    set(__so_suffix__ ".${__python_soabi__}")
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Darwin" OR CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set_target_properties(pypplcommon_shared PROPERTIES SUFFIX "${__so_suffix__}.so") # using .so instead of .dylib on macosx
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set_target_properties(pypplcommon_shared PROPERTIES SUFFIX "${__so_suffix__}.pyd") # using .pyd instead of .dll
endif()

unset(__so_suffix__)
unset(__python_soabi__)

if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    target_link_options(pypplcommon_shared PUBLIC "LINKER:-undefined,dynamic_lookup") # required by pybind11
endif()

if(PPLCOMMON_INSTALL)
    install(TARGETS pypplcommon_shared DESTINATION lib/pyppl)
endif()
