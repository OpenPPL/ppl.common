#include "memory.h"
#include "device.h"
#include "ppl/common/log.h"
namespace ppl { namespace common { namespace ocl {

RetCode OpenclMemeoryAlloc(cl_command_queue queue,ImgDesc_t* memDesc,cl_mem mem){
    cl_context ctx;
    cl_int rc = clGetCommandQueueInfo(queue, CL_QUEUE_CONTEXT, sizeof(cl_context), &ctx, nullptr);
    if(CL_SUCCESS != rc) {
        LOG(ERROR) << "Call clGetCommandQueueInfo failed with code: " << rc;
        return rc;
    }
    if(memDesc->mem_type == PPL_OCL_IMAGE && memDesc->width< GetMaxImageWidth() && memDesc->height<GetMaxImageHeight() &&
    memDesc->depth<GetMaxImageDepth()){
        cl_image_desc d;
        d.image_type        = memDesc->image_type;
        d.image_width       = memDesc->width;
        d.image_height      = memDesc->height;
        d.image_depth       = memDesc->depth;
        d.image_array_size  = 0;
        d.image_row_pitch   = 0;
        d.image_slice_pitch = 0;
        d.num_mip_levels    = 0;
        d.num_samples       = 0;
        d.buffer            = nullptr;
        cl_image_format fmt = {memDesc->channel_order, memDesc->data_type};
        mem = clCreateImage(ctx, memDesc->mem_flags, &fmt, &d, nullptr, &rc);
    }else{
        size_t memByteSize = memDesc->width * memDesc->height * memDesc->depth;
        int dataTypeSize = (memDesc->data_type == CL_HALF_FLOAT) ? 2:(memDesc->data_type == CL_SIGNED_INT8 ? 1 : 4);
        int elementNum = (memDesc->channel_order == CL_RG) ? 2:(memDesc->data_type == CL_R ? 1 : 4);
        memByteSize *= (dataTypeSize*elementNum);
        if(memDesc->byteSize != 0)
            memByteSize = memDesc->byteSize;
        mem = clCreateBuffer(ctx, memDesc->mem_flags, memByteSize, nullptr, &rc);
        memDesc->mem_type = PPL_OCL_BUFFER;
    }
    if(rc != CL_SUCCESS) return RC_HOST_MEMORY_ERROR;
    return RC_SUCCESS;
}

}}}