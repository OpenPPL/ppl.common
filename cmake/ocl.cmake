if(NOT CL_TARGET_OPENCL_VERSION)
    message(FATAL_ERROR "`CL_TARGET_OPENCL_VERSION` is not specified.")
endif()

hpcc_populate_dep(opencl_headers)

file(GLOB_RECURSE __PPLCOMMON_OCL_SRC__ src/ppl/common/ocl/*.cc)
list(APPEND PPLCOMMON_SRC ${__PPLCOMMON_OCL_SRC__})
unset(__PPLCOMMON_OCL_SRC__)

list(APPEND PPLCOMMON_INCLUDES ${HPCC_DEPS_DIR}/opencl_headers)

if(NOT CMAKE_SYSTEM_NAME STREQUAL "QNX" AND NOT MSVC)
    list(APPEND PPLCOMMON_LINK_LIBRARIES dl)
endif()

list(APPEND PPLCOMMON_DEFINITIONS
    PPLCOMMON_USE_OPENCL
    CL_TARGET_OPENCL_VERSION=${CL_TARGET_OPENCL_VERSION})

if(PPLCOMMON_BUILD_OPENCL_TOOLS)
    if(NOT PPLCOMMON_OPENCL_LIBRARIES)
        message(FATAL_ERROR "`PPLCOMMON_OPENCL_LIBRARIES` is not specified.")
    endif()
    if(NOT TARGET oclgpuinfo)
        add_executable(oclgpuinfo tools/ocl/oclgpuinfo.cc)
        target_link_libraries(oclgpuinfo PRIVATE pplcommon_static ${PPLCOMMON_OPENCL_LIBRARIES})
    endif()
endif()

# ----- installation ----- #

if(PPLCOMMON_INSTALL)
    file(GLOB __OCL_HEADERS__ src/ppl/common/ocl/*.h)
    install(FILES ${__OCL_HEADERS__} DESTINATION include/ppl/common/ocl)
    unset(__OCL_HEADERS__)
endif()
