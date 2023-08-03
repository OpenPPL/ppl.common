#ifndef _ST_HPC_PPL_COMMON_OCL_MEMORY_H_
#define _ST_HPC_PPL_COMMON_OCL_MEMORY_H_

#include "ppl/common/retcode.h"
#include "ppl/common/ocl/memory_option.h"

namespace ppl { namespace common { namespace ocl {

RetCode OpenclMemeoryAlloc(cl_command_queue queue,ImgDesc_t* memDesc,cl_mem mem);


}}}

#endif