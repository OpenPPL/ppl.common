#include "memory.h"
#include "device.h"
#include "ppl/common/log.h"
namespace ppl { namespace common { namespace ocl {

cl_mem OpenCLMemeoryAlloc(cl_context ctx, uint64_t size, cl_int* err, cl_mem_flags flags) {
    cl_int rc = 0;
    cl_mem buffer = clCreateBuffer(ctx, flags, size, nullptr, &rc);
    if (CL_SUCCESS != rc) {
        LOG(ERROR) << "Call clCreateBuffer failed with code: " << rc;
        *err = rc;
        return nullptr;
    }

    *err = rc;
    return buffer;
}

}}} // namespace ppl::common::ocl