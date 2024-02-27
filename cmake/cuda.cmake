option(PPLCOMMON_ENABLE_NCCL "" OFF)

include(${HPCC_DEPS_DIR}/hpcc/cmake/cuda-common.cmake)

file(GLOB __SRC__ src/ppl/common/cuda/*.cc)
list(APPEND PPLCOMMON_SRC ${__SRC__})
unset(__SRC__)

list(APPEND PPLCOMMON_LINK_LIBRARIES cuda cudart_static)
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    list(APPEND PPLCOMMON_LINK_LIBRARIES dl rt) # required by cudart_static
endif()

list(APPEND PPLCOMMON_INCLUDES ${CMAKE_CUDA_TOOLKIT_INCLUDE_DIRECTORIES})
list(APPEND PPLCOMMON_LINK_DIRECTORIES ${CMAKE_CUDA_HOST_IMPLICIT_LINK_DIRECTORIES})

if (PPLCOMMON_ENABLE_NCCL)
    if(PPLCOMMON_ENABLE_SYSTEM_NCCL)
        find_path(NCCL_INCLUDE_DIRS NAMES nccl.h)
        find_library(NCCL_LIBRARIES NAMES nccl_static)
    else()
        hpcc_populate_dep(nccl)

        if(NOT PPLCOMMON_CUDA_ARCHITECTURES)
            set(PPLCOMMON_CUDA_ARCHITECTURES "80;86;87")
        endif()

        set(__NVCC_GENCODE__ )
        foreach(arch ${PPLCOMMON_CUDA_ARCHITECTURES})
            set(__NVCC_GENCODE__ "${__NVCC_GENCODE__} -gencode arch=compute_${arch},code=sm_${arch}")
        endforeach()

        set(NCCL_INCLUDE_DIRS ${nccl_BINARY_DIR}/include)
        set(NCCL_LIBRARIES ${nccl_BINARY_DIR}/lib/libnccl_static.a)

        include(ProcessorCount)
        ProcessorCount(__NPROC__)
        execute_process(COMMAND make NVCC_GENCODE=${__NVCC_GENCODE__} BUILDDIR=${nccl_BINARY_DIR} -j${__NPROC__} -C ${HPCC_DEPS_DIR}/nccl src.build)
        unset(__NPROC__)

        unset(__NVCC_GENCODE__)
    endif()

    list(APPEND PPLCOMMON_INCLUDES ${NCCL_INCLUDE_DIRS})
    list(APPEND PPLCOMMON_LINK_LIBRARIES ${NCCL_LIBRARIES})
    list(APPEND PPLCOMMON_DEFINITIONS PPLCOMMON_ENABLE_NCCL)

    if(PPLCOMMON_INSTALL)
        install(FILES ${NCCL_INCLUDE_DIRS}/nccl.h ${NCCL_INCLUDE_DIRS}/nccl_net.h DESTINATION include)
        install(FILES ${NCCL_LIBRARIES} DESTINATION lib)
        unset(__NCCL_HEADERS__)
    endif()
endif()

if(PPLCOMMON_INSTALL)
    file(GLOB PPLCOMMON_CUDA_HEADERS src/ppl/common/cuda/*.h)
    install(FILES ${PPLCOMMON_CUDA_HEADERS} DESTINATION include/ppl/common/cuda)
endif()
