#ifndef _ST_HPC_PPL_COMMON_OCL_MEMORY_H_
#define _ST_HPC_PPL_COMMON_OCL_MEMORY_H_

#include "ppl/common/retcode.h"
#include "ppl/common/ocl/memory_option.h"

namespace ppl { namespace common { namespace ocl {

cl_mem OpenCLMemeoryAlloc(cl_context ctx, uint64_t size, cl_int* err, cl_mem_flags flags = CL_MEM_READ_WRITE);

}}} // namespace ppl::common::ocl

#endif