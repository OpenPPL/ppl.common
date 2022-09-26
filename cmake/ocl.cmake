list(APPEND PPLCOMMON_DEFINITIONS PPLCOMMON_USE_OCL)

file(GLOB_RECURSE PPLCOMMON_OCL_SRC src/ppl/common/ocl/*.cc)

list(APPEND PPLCOMMON_SRC ${PPLCOMMON_OCL_SRC})
list(APPEND PPLCOMMON_LINK_LIBRARIES OpenCL)

if(NOT TARGET oclgpuinfo)
    add_executable(oclgpuinfo tools/ocl/oclgpuinfo.cc)
    target_link_libraries(oclgpuinfo PRIVATE pplcommon_static)
endif()

# ----- installation ----- #

file(GLOB PPLCOMMON_OCL_HEADERS src/ppl/common/ocl/*.h)
install(FILES ${PPLCOMMON_OCL_HEADERS} DESTINATION include/ppl/common/ocl)
