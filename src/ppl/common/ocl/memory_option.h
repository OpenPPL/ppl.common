#ifndef __ST_HPC_PPL_COMMON_OCL_MEMORY_OPTION_H_
#define __ST_HPC_PPL_COMMON_OCL_MEMORY_OPTION_H_

#include "openclruntime.h"
namespace ppl { namespace common { namespace ocl {

#define PPL_OCL_IMAGE 0 
#define PPL_OCL_BUFFER 1

#define OCL_INT8 0
#define OCL_FP16 1
#define OCL_FP32 2

typedef struct ImgDesc {
    ImgDesc(){}
    size_t width = 1;
    size_t height = 1;
    size_t depth = 1;
    size_t byteSize = 0;
    int mem_type = PPL_OCL_IMAGE;
    cl_channel_type     data_type       = CL_HALF_FLOAT;
    cl_channel_order    channel_order   = CL_RGBA;
    cl_mem_flags        mem_flags       = CL_MEM_READ_WRITE ;
    cl_mem_object_type  image_type      = CL_MEM_OBJECT_IMAGE3D;
} ImgDesc_t;

}}}
#endif