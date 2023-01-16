if(NOT PPLCOMMON_OPENCL_INCLUDE_DIRS)
    message(FATAL_ERROR "`PPLCOMMON_OPENCL_INCLUDE_DIRS` is not specified.")
endif()
if(NOT PPLCOMMON_OPENCL_LIBRARIES)
    message(FATAL_ERROR "`PPLCOMMON_OPENCL_LIBRARIES` is not specified.")
endif()
if(NOT CL_TARGET_OPENCL_VERSION)
    message(FATAL_ERROR "`CL_TARGET_OPENCL_VERSION` is not specified.")
endif()

file(GLOB_RECURSE __PPLCOMMON_OCL_SRC__ src/ppl/common/ocl/*.cc)
list(APPEND PPLCOMMON_SRC ${__PPLCOMMON_OCL_SRC__})
unset(__PPLCOMMON_OCL_SRC__)

list(APPEND PPLCOMMON_INCLUDES ${PPLCOMMON_OPENCL_INCLUDE_DIRS})
list(APPEND PPLCOMMON_LINK_LIBRARIES ${PPLCOMMON_OPENCL_LIBRARIES})

list(APPEND PPLCOMMON_DEFINITIONS
    PPLCOMMON_USE_OCL
    CL_TARGET_OPENCL_VERSION=${CL_TARGET_OPENCL_VERSION})

if(NOT TARGET oclgpuinfo)
    add_executable(oclgpuinfo tools/ocl/oclgpuinfo.cc)
    target_link_libraries(oclgpuinfo PRIVATE pplcommon_static)
endif()

# ----- installation ----- #

if(PPLCOMMON_INSTALL)
    file(GLOB __OCL_HEADERS__ src/ppl/common/ocl/*.h)
    install(FILES ${__OCL_HEADERS__} DESTINATION include/ppl/common/ocl)
    unset(__OCL_HEADERS__)
endif()
