#ifndef __ST_HPC_PPL_COMMON_OCL_OPENCLRUNTIME_H_
#define __ST_HPC_PPL_COMMON_OCL_OPENCLRUNTIME_H_
#include "CL/cl.h"
#include "CL/cl_ext.h"

namespace ppl { namespace common { namespace ocl {

class OpenCLAPI {
    void* libOpenCL_;

public:
    explicit OpenCLAPI(const char* libFileName = nullptr);
    ~OpenCLAPI();
    static OpenCLAPI* instance();

    cl_int ReleaseContextInternel_(cl_context ctx);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#define DECLARE_CL_SYMBOL_PTR(NAME) decltype(cl##NAME)* NAME##_
    DECLARE_CL_SYMBOL_PTR(BuildProgram);
    DECLARE_CL_SYMBOL_PTR(CreateBuffer);
    DECLARE_CL_SYMBOL_PTR(CreateCommandQueue);
    DECLARE_CL_SYMBOL_PTR(CreateCommandQueueWithProperties);
    DECLARE_CL_SYMBOL_PTR(CreateContext);
    DECLARE_CL_SYMBOL_PTR(CreateContextFromType);
    DECLARE_CL_SYMBOL_PTR(CreateImage2D);
    DECLARE_CL_SYMBOL_PTR(CreateImage3D);
    DECLARE_CL_SYMBOL_PTR(CreateKernel);
    cl_kernel (*CloneKernel_)(cl_kernel, cl_int*) = nullptr; // explicitly disable clCloneKernel
    DECLARE_CL_SYMBOL_PTR(CreateKernelsInProgram);
    DECLARE_CL_SYMBOL_PTR(CreateProgramWithBinary);
    DECLARE_CL_SYMBOL_PTR(CreateProgramWithSource);
    DECLARE_CL_SYMBOL_PTR(CreateSampler);
    DECLARE_CL_SYMBOL_PTR(CreateSubBuffer);
    DECLARE_CL_SYMBOL_PTR(CreateUserEvent);
    DECLARE_CL_SYMBOL_PTR(EnqueueBarrier);
    DECLARE_CL_SYMBOL_PTR(EnqueueCopyBuffer);
    DECLARE_CL_SYMBOL_PTR(EnqueueCopyBufferRect);
    DECLARE_CL_SYMBOL_PTR(EnqueueCopyBufferToImage);
    DECLARE_CL_SYMBOL_PTR(EnqueueCopyImage);
    DECLARE_CL_SYMBOL_PTR(EnqueueCopyImageToBuffer);
    DECLARE_CL_SYMBOL_PTR(EnqueueMapBuffer);
    DECLARE_CL_SYMBOL_PTR(EnqueueMapImage);
    DECLARE_CL_SYMBOL_PTR(EnqueueMarker);
    DECLARE_CL_SYMBOL_PTR(EnqueueNativeKernel);
    DECLARE_CL_SYMBOL_PTR(EnqueueNDRangeKernel);
    DECLARE_CL_SYMBOL_PTR(EnqueueReadBuffer);
    DECLARE_CL_SYMBOL_PTR(EnqueueReadBufferRect);
    DECLARE_CL_SYMBOL_PTR(EnqueueReadImage);
    DECLARE_CL_SYMBOL_PTR(EnqueueTask);
    DECLARE_CL_SYMBOL_PTR(EnqueueUnmapMemObject);
    DECLARE_CL_SYMBOL_PTR(EnqueueWaitForEvents);
    DECLARE_CL_SYMBOL_PTR(EnqueueWriteBuffer);
    DECLARE_CL_SYMBOL_PTR(EnqueueWriteBufferRect);
    DECLARE_CL_SYMBOL_PTR(EnqueueWriteImage);
    DECLARE_CL_SYMBOL_PTR(Finish);
    DECLARE_CL_SYMBOL_PTR(Flush);
    DECLARE_CL_SYMBOL_PTR(GetCommandQueueInfo);
    DECLARE_CL_SYMBOL_PTR(GetContextInfo);
    DECLARE_CL_SYMBOL_PTR(GetDeviceIDs);
    DECLARE_CL_SYMBOL_PTR(GetDeviceInfo);
    DECLARE_CL_SYMBOL_PTR(GetEventInfo);
    DECLARE_CL_SYMBOL_PTR(GetEventProfilingInfo);
    DECLARE_CL_SYMBOL_PTR(GetImageInfo);
    DECLARE_CL_SYMBOL_PTR(GetKernelInfo);
    DECLARE_CL_SYMBOL_PTR(GetKernelWorkGroupInfo);
    DECLARE_CL_SYMBOL_PTR(GetMemObjectInfo);
    DECLARE_CL_SYMBOL_PTR(GetPlatformIDs);
    DECLARE_CL_SYMBOL_PTR(GetPlatformInfo);
    DECLARE_CL_SYMBOL_PTR(GetProgramBuildInfo);
    DECLARE_CL_SYMBOL_PTR(GetProgramInfo);
    DECLARE_CL_SYMBOL_PTR(GetSamplerInfo);
    DECLARE_CL_SYMBOL_PTR(GetSupportedImageFormats);
    DECLARE_CL_SYMBOL_PTR(ReleaseCommandQueue);
    DECLARE_CL_SYMBOL_PTR(ReleaseContext);
    DECLARE_CL_SYMBOL_PTR(ReleaseEvent);
    DECLARE_CL_SYMBOL_PTR(ReleaseKernel);
    DECLARE_CL_SYMBOL_PTR(ReleaseMemObject);
    DECLARE_CL_SYMBOL_PTR(ReleaseProgram);
    DECLARE_CL_SYMBOL_PTR(ReleaseSampler);
    DECLARE_CL_SYMBOL_PTR(RetainCommandQueue);
    DECLARE_CL_SYMBOL_PTR(RetainContext);
    DECLARE_CL_SYMBOL_PTR(RetainEvent);
    DECLARE_CL_SYMBOL_PTR(RetainKernel);
    DECLARE_CL_SYMBOL_PTR(RetainMemObject);
    DECLARE_CL_SYMBOL_PTR(RetainProgram);
    DECLARE_CL_SYMBOL_PTR(RetainSampler);
    DECLARE_CL_SYMBOL_PTR(SetEventCallback);
    DECLARE_CL_SYMBOL_PTR(SetKernelArg);
    DECLARE_CL_SYMBOL_PTR(SetUserEventStatus);
    DECLARE_CL_SYMBOL_PTR(WaitForEvents);
    DECLARE_CL_SYMBOL_PTR(RetainDevice);
    DECLARE_CL_SYMBOL_PTR(ReleaseDevice);
    DECLARE_CL_SYMBOL_PTR(CreateImage);
    DECLARE_CL_SYMBOL_PTR(EnqueueFillBuffer);
    DECLARE_CL_SYMBOL_PTR(EnqueueFillImage);
    DECLARE_CL_SYMBOL_PTR(GetDeviceImageInfoQCOM);
    DECLARE_CL_SYMBOL_PTR(SVMAlloc);
    DECLARE_CL_SYMBOL_PTR(SVMFree);
    DECLARE_CL_SYMBOL_PTR(EnqueueSVMFree);
    DECLARE_CL_SYMBOL_PTR(EnqueueSVMMemcpy);
    DECLARE_CL_SYMBOL_PTR(EnqueueSVMMemFill);
    DECLARE_CL_SYMBOL_PTR(EnqueueSVMMap);
    DECLARE_CL_SYMBOL_PTR(EnqueueSVMUnmap);
    DECLARE_CL_SYMBOL_PTR(SetKernelArgSVMPointer);
    DECLARE_CL_SYMBOL_PTR(GetKernelSubGroupInfoKHR);

#pragma GCC diagnostic pop
#undef DECLARE_CL_SYMBOL_PTR

private:
    bool isVndkSupportUsed_ = false;
};

// Overriding
#define clBuildProgram opencl_api->BuildProgram_
#define clCreateBuffer opencl_api->CreateBuffer_
#define clCreateCommandQueue opencl_api->CreateCommandQueue_
#define clCreateCommandQueueWithProperties opencl_api->CreateCommandQueueWithProperties_
#define clCreateContext opencl_api->CreateContext_
#define clCreateContextFromType opencl_api->CreateContextFromType_
#define clCreateImage2D opencl_api->CreateImage2D_
#define clCreateImage3D opencl_api->CreateImage3D_
#define clCreateKernel opencl_api->CreateKernel_
#define clCloneKernel opencl_api->CloneKernel_
#define clCreateKernelsInProgram opencl_api->CreateKernelsInProgram_
#define clCreateProgramWithBinary opencl_api->CreateProgramWithBinary_
#define clCreateProgramWithSource opencl_api->CreateProgramWithSource_
#define clCreateSampler opencl_api->CreateSampler_
#define clCreateSubBuffer opencl_api->CreateSubBuffer_
#define clCreateUserEvent opencl_api->CreateUserEvent_
#define clEnqueueBarrier opencl_api->EnqueueBarrier_
#define clEnqueueCopyBuffer opencl_api->EnqueueCopyBuffer_
#define clEnqueueCopyBufferRect opencl_api->EnqueueCopyBufferRect_
#define clEnqueueCopyBufferToImage opencl_api->EnqueueCopyBufferToImage_
#define clEnqueueCopyImage opencl_api->EnqueueCopyImage_
#define clEnqueueCopyImageToBuffer opencl_api->EnqueueCopyImageToBuffer_
#define clEnqueueMapBuffer opencl_api->EnqueueMapBuffer_
#define clEnqueueMapImage opencl_api->EnqueueMapImage_
#define clEnqueueMarker opencl_api->EnqueueMarker_
#define clEnqueueNativeKernel opencl_api->EnqueueNativeKernel_
#define clEnqueueNDRangeKernel opencl_api->EnqueueNDRangeKernel_
#define clEnqueueReadBuffer opencl_api->EnqueueReadBuffer_
#define clEnqueueReadBufferRect opencl_api->EnqueueReadBufferRect_
#define clEnqueueReadImage opencl_api->EnqueueReadImage_
#define clEnqueueTask opencl_api->EnqueueTask_
#define clEnqueueUnmapMemObject opencl_api->EnqueueUnmapMemObject_
#define clEnqueueWaitForEvents opencl_api->EnqueueWaitForEvents_
#define clEnqueueWriteBuffer opencl_api->EnqueueWriteBuffer_
#define clEnqueueWriteBufferRect opencl_api->EnqueueWriteBufferRect_
#define clEnqueueWriteImage opencl_api->EnqueueWriteImage_
#define clFinish opencl_api->Finish_
#define clFlush opencl_api->Flush_
#define clGetCommandQueueInfo opencl_api->GetCommandQueueInfo_
#define clGetContextInfo opencl_api->GetContextInfo_
#define clGetDeviceIDs opencl_api->GetDeviceIDs_
#define clGetDeviceInfo opencl_api->GetDeviceInfo_
#define clGetEventInfo opencl_api->GetEventInfo_
#define clGetEventProfilingInfo opencl_api->GetEventProfilingInfo_
#define clGetImageInfo opencl_api->GetImageInfo_
#define clGetKernelInfo opencl_api->GetKernelInfo_
#define clGetKernelWorkGroupInfo opencl_api->GetKernelWorkGroupInfo_
#define clGetMemObjectInfo opencl_api->GetMemObjectInfo_
#define clGetPlatformIDs opencl_api->GetPlatformIDs_
#define clGetPlatformInfo opencl_api->GetPlatformInfo_
#define clGetProgramBuildInfo opencl_api->GetProgramBuildInfo_
#define clGetProgramInfo opencl_api->GetProgramInfo_
#define clGetSamplerInfo opencl_api->GetSamplerInfo_
#define clGetSupportedImageFormats opencl_api->GetSupportedImageFormats_
#define clReleaseCommandQueue opencl_api->ReleaseCommandQueue_
#define clReleaseContext opencl_api->ReleaseContext_
#define clReleaseEvent opencl_api->ReleaseEvent_
#define clReleaseKernel opencl_api->ReleaseKernel_
#define clReleaseMemObject opencl_api->ReleaseMemObject_
#define clReleaseProgram opencl_api->ReleaseProgram_
#define clReleaseSampler opencl_api->ReleaseSampler_
#define clRetainCommandQueue opencl_api->RetainCommandQueue_
#define clRetainContext opencl_api->RetainContext_
#define clRetainEvent opencl_api->RetainEvent_
#define clRetainKernel opencl_api->RetainKernel_
#define clRetainMemObject opencl_api->RetainMemObject_
#define clRetainProgram opencl_api->RetainProgram_
#define clRetainSampler opencl_api->RetainSampler_
#define clSetEventCallback opencl_api->SetEventCallback_
#define clSetKernelArg opencl_api->SetKernelArg_
#define clSetUserEventStatus opencl_api->SetUserEventStatus_
#define clWaitForEvents opencl_api->WaitForEvents_
#define clCreateImage opencl_api->CreateImage_
#define clEnqueueFillBuffer opencl_api->EnqueueFillBuffer_
#define clEnqueueFillImage opencl_api->EnqueueFillImage_
#define clGetDeviceImageInfoQCOM opencl_api->GetDeviceImageInfoQCOM_
#define clReleaseDevice opencl_api->ReleaseDevice_
#define clRetainDevice opencl_api->RetainDevice_
#define clSVMAlloc opencl_api->SVMAlloc_
#define clSVMFree opencl_api->SVMFree_
#define clEnqueueSVMFree opencl_api->EnqueueSVMFree_
#define clEnqueueSVMMemcpy opencl_api->EnqueueSVMMemcpy_
#define clEnqueueSVMMemFill opencl_api->EnqueueSVMMemFill_
#define clEnqueueSVMMap opencl_api->EnqueueSVMMap_
#define clEnqueueSVMUnmap opencl_api->EnqueueSVMUnmap_
#define clSetKernelArgSVMPointer opencl_api->SetKernelArgSVMPointer_
#define clGetKernelSubGroupInfoKHR opencl_api->GetKernelSubGroupInfoKHR_

#define opencl_api ::ppl::common::ocl::OpenCLAPI::instance()

}}} // namespace ppl::common::ocl

#endif // __ST_HPC_PPL3_CORE_OCL_OPENCLAPI_H_
