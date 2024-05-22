#include "openclruntime.h"
#include "ppl/common/log.h"

#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <memory>

#ifndef _WIN32
#include <dlfcn.h>
extern "C" void* android_load_sphal_library(const char* name, int flag);
extern "C" int android_unload_sphal_library(void* handel);
#define PPL_SYS_TYPE void*
#define PPL_OPENCL_LIB_FUNC dlsym
#else
#include <windows.h>
#define PPL_SYS_TYPE HMODULE
#define PPL_OPENCL_LIB_FUNC GetProcAddress
#endif

namespace ppl { namespace common { namespace ocl {

OpenCLAPI* OpenCLAPI::instance() {
    static OpenCLAPI sInstance;
    return &sInstance;
}

#ifndef _WIN32
static const char* kAlternativeCLLibs[] = {
    nullptr,
    nullptr,
    "libOpenCL.so",
    "libPVROCL.so",
    "libGLES_mali.so",
#if defined(__arm64__) || defined(__aarch64__)
    "/system/vendor/lib64/libOpenCL.so",
    "/system/vendor/lib64/egl/libGLES_mali.so",
    "/system/vendor/lib/libPVROCL.so",
#elif defined(__arm__)
    "/system/vendor/lib/libOpenCL.so",
    "/system/vendor/lib/egl/libGLES_mali.so",
    "/system/vendor/lib/libPVROCL.so",
#endif
};

namespace {
class libvndksupport {
public:
    libvndksupport()
        : libvndksupport_(nullptr), android_load_sphal_library_(nullptr), android_unload_sphal_library_(nullptr) {
#if defined(__ANDROID__) || defined(ANDROID)
        if (void* lib = dlopen("libvndksupport.so", RTLD_NOW | RTLD_LOCAL)) {
            this->libvndksupport_ = lib;
            this->android_load_sphal_library_ =
                (decltype(this->android_load_sphal_library_))dlsym(lib, "android_load_sphal_library");
            this->android_unload_sphal_library_ =
                (decltype(this->android_unload_sphal_library_))dlsym(lib, "android_unload_sphal_library");
        }
#endif // android has libvnksupport.so
        if (!this->android_load_sphal_library_ || !this->android_unload_sphal_library_) {
            this->android_load_sphal_library_ = &dlopen;
            this->android_unload_sphal_library_ = &dlclose;
        }
    }
    void* load(const char* libFileName, int flag) {
        return this->android_load_sphal_library_(libFileName, flag);
    }
    int unload(void* lib) {
        return this->android_unload_sphal_library_(lib);
    }
    ~libvndksupport() {
        if (this->libvndksupport_) {
            int rc = dlclose(this->libvndksupport_);
            if (rc != 0)
                LOG(ERROR) << "libvndksupport: " << dlerror();
        }
    }
    static libvndksupport* instance();

private:
    void* libvndksupport_;
    decltype(&android_load_sphal_library) android_load_sphal_library_;
    decltype(&android_unload_sphal_library) android_unload_sphal_library_;
};
libvndksupport* libvndksupport::instance() {
    static libvndksupport sInstance;
    return &sInstance;
}
} // namespace

static void* try_libOpenCLPixel() {
#ifdef __aarch64__
    const char* libOpenCLPixelPath = "/system/vendor/lib64/libOpenCL-pixel.so";
#else
    const char* libOpenCLPixelPath = "/system/vendor/lib/libOpenCL-pixel.so";
#endif
    void* libOpenCLPixel = dlopen(libOpenCLPixelPath, RTLD_NOW | RTLD_LOCAL);
    if (!libOpenCLPixel) {
        LOG(DEBUG) << "dlopen: " << dlerror();
        return nullptr;
    }
    void* (*pfn_enableOpenCL)() = nullptr;
    *((void**)&pfn_enableOpenCL) = dlsym(libOpenCLPixel, "enableOpenCL");
    if (!pfn_enableOpenCL) {
        dlclose(libOpenCLPixel);
        return nullptr;
    }
    void* ret = pfn_enableOpenCL();
    dlclose(libOpenCLPixel);
    return ret;
}
#endif

OpenCLAPI::OpenCLAPI(const char* libFileName) {
    //! make sure that the kernel pool is constructed before the OpenCLAPI
    memset(this, 0, sizeof(*this));
#ifndef _WIN32
    void* lib = try_libOpenCLPixel();
    if (lib) {
        this->libOpenCL_ = lib;
        goto LABEL_LOAD_CL_SYMBOLS;
    }
    kAlternativeCLLibs[0] = libFileName;
    kAlternativeCLLibs[1] = getenv("PPL3CORE_OPENCL_LIBRARY");
    for (size_t i = 0; i < sizeof(kAlternativeCLLibs) / sizeof(char*); ++i) {
        const char* altLib = kAlternativeCLLibs[i];
        if (altLib == nullptr) {
            continue;
        }
        if (void* lib = libvndksupport::instance()->load(altLib, RTLD_NOW | RTLD_LOCAL)) {
            this->libOpenCL_ = lib;
            this->isVndkSupportUsed_ = true;
            LOG(DEBUG) << "load " << altLib << " by vndksupport ok: " << lib;
            goto LABEL_LOAD_CL_SYMBOLS;
        } else {
            LOG(INFO) << "load " << altLib << " failed: " << dlerror();
        }
    }
    LOG(ERROR) << "There is no available OpenCL library! You may set environment variable \"PPL3CORE_OPENCL_LIBRARY\".";

#else
    const char* envPath = getenv("PPL3CORE_OPENCL_LIBRARY");
    const char* defaultPath = (envPath ? envPath : "OpenCL.dll");
    if (void* lib = LoadLibrary(defaultPath)) {
        this->libOpenCL_ = lib;
        goto LABEL_LOAD_CL_SYMBOLS;
    }
    LOG(ERROR) << "There is no available OpenCL library! You may set environment variable \"PPL3CORE_OPENCL_LIBRARY\".";
#endif
    return;
LABEL_LOAD_CL_SYMBOLS:
#define LOAD_CL_SYMBOL(NAME)                                                                                        \
    do {                                                                                                            \
        this->NAME##_ = (decltype(this->NAME##_))PPL_OPENCL_LIB_FUNC((PPL_SYS_TYPE)(this->libOpenCL_), "cl" #NAME); \
        if (NULL == this->NAME##_)                                                                                  \
            LOG(ERROR) << #NAME << "is null";                                                                       \
    } while (0)
    LOAD_CL_SYMBOL(BuildProgram);
    LOAD_CL_SYMBOL(CreateBuffer);
    LOAD_CL_SYMBOL(CreateCommandQueue);
    LOAD_CL_SYMBOL(CreateCommandQueueWithProperties);
    LOAD_CL_SYMBOL(CreateContext);
    LOAD_CL_SYMBOL(CreateImage2D);
    LOAD_CL_SYMBOL(CreateImage3D);
    LOAD_CL_SYMBOL(CreateKernel);
    // LOAD_CL_SYMBOL(CloneKernel);
    LOAD_CL_SYMBOL(CreateKernelsInProgram);
    LOAD_CL_SYMBOL(CreateProgramWithBinary);
    LOAD_CL_SYMBOL(CreateProgramWithSource);
    LOAD_CL_SYMBOL(CreateSampler);
    LOAD_CL_SYMBOL(CreateSubBuffer);
    LOAD_CL_SYMBOL(CreateUserEvent);
    LOAD_CL_SYMBOL(EnqueueBarrier);
    LOAD_CL_SYMBOL(EnqueueCopyBuffer);
    LOAD_CL_SYMBOL(EnqueueCopyBufferRect);
    LOAD_CL_SYMBOL(EnqueueCopyBufferToImage);
    LOAD_CL_SYMBOL(EnqueueCopyImage);
    LOAD_CL_SYMBOL(EnqueueCopyImageToBuffer);
    LOAD_CL_SYMBOL(EnqueueMapBuffer);
    LOAD_CL_SYMBOL(EnqueueMapImage);
    LOAD_CL_SYMBOL(EnqueueMarker);
    LOAD_CL_SYMBOL(EnqueueNativeKernel);
    LOAD_CL_SYMBOL(EnqueueNDRangeKernel);
    LOAD_CL_SYMBOL(EnqueueReadBuffer);
    LOAD_CL_SYMBOL(EnqueueReadBufferRect);
    LOAD_CL_SYMBOL(EnqueueReadImage);
    LOAD_CL_SYMBOL(EnqueueTask);
    LOAD_CL_SYMBOL(EnqueueUnmapMemObject);
    LOAD_CL_SYMBOL(EnqueueWaitForEvents);
    LOAD_CL_SYMBOL(EnqueueWriteBuffer);
    LOAD_CL_SYMBOL(EnqueueWriteBufferRect);
    LOAD_CL_SYMBOL(EnqueueWriteImage);
    LOAD_CL_SYMBOL(Finish);
    LOAD_CL_SYMBOL(Flush);
    LOAD_CL_SYMBOL(GetCommandQueueInfo);
    LOAD_CL_SYMBOL(GetContextInfo);
    LOAD_CL_SYMBOL(GetDeviceIDs);
    LOAD_CL_SYMBOL(GetDeviceInfo);
    LOAD_CL_SYMBOL(GetEventInfo);
    LOAD_CL_SYMBOL(GetEventProfilingInfo);
    LOAD_CL_SYMBOL(GetImageInfo);
    LOAD_CL_SYMBOL(GetKernelInfo);
    LOAD_CL_SYMBOL(GetKernelWorkGroupInfo);
    LOAD_CL_SYMBOL(GetMemObjectInfo);
    LOAD_CL_SYMBOL(GetPlatformIDs);
    LOAD_CL_SYMBOL(GetPlatformInfo);
    LOAD_CL_SYMBOL(GetProgramBuildInfo);
    LOAD_CL_SYMBOL(GetProgramInfo);
    LOAD_CL_SYMBOL(GetSamplerInfo);
    LOAD_CL_SYMBOL(GetSupportedImageFormats);
    LOAD_CL_SYMBOL(ReleaseCommandQueue);
    LOAD_CL_SYMBOL(ReleaseContext);
    LOAD_CL_SYMBOL(ReleaseEvent);
    LOAD_CL_SYMBOL(ReleaseKernel);
    LOAD_CL_SYMBOL(ReleaseMemObject);
    LOAD_CL_SYMBOL(ReleaseProgram);
    LOAD_CL_SYMBOL(ReleaseSampler);
    LOAD_CL_SYMBOL(RetainCommandQueue);
    LOAD_CL_SYMBOL(RetainContext);
    LOAD_CL_SYMBOL(RetainEvent);
    LOAD_CL_SYMBOL(RetainKernel);
    LOAD_CL_SYMBOL(RetainMemObject);
    LOAD_CL_SYMBOL(RetainProgram);
    LOAD_CL_SYMBOL(RetainSampler);
    LOAD_CL_SYMBOL(SetEventCallback);
    LOAD_CL_SYMBOL(SetKernelArg);
    LOAD_CL_SYMBOL(SetUserEventStatus);
    LOAD_CL_SYMBOL(WaitForEvents);
    LOAD_CL_SYMBOL(CreateImage);
    LOAD_CL_SYMBOL(RetainDevice);
    LOAD_CL_SYMBOL(ReleaseDevice);
    LOAD_CL_SYMBOL(EnqueueFillBuffer);
    LOAD_CL_SYMBOL(EnqueueFillImage);
    LOAD_CL_SYMBOL(GetDeviceImageInfoQCOM);
    LOAD_CL_SYMBOL(SVMAlloc);
    LOAD_CL_SYMBOL(SVMFree);
    LOAD_CL_SYMBOL(EnqueueSVMFree);
    LOAD_CL_SYMBOL(EnqueueSVMMemcpy);
    LOAD_CL_SYMBOL(EnqueueSVMMemFill);
    LOAD_CL_SYMBOL(EnqueueSVMMap);
    LOAD_CL_SYMBOL(EnqueueSVMUnmap);
    LOAD_CL_SYMBOL(SetKernelArgSVMPointer);
#undef LOAD_CL_SYMBOL
}

OpenCLAPI::~OpenCLAPI() {
    if (this->libOpenCL_) {
        if (this->isVndkSupportUsed_) {
#ifndef _WIN32
            int rc = libvndksupport::instance()->unload(this->libOpenCL_);
            if (rc != 0)
                LOG(ERROR) << "unload: " << dlerror();
#endif
        } else {
#ifndef _WIN32
            int rc = dlclose(this->libOpenCL_);
            if (rc != 0)
                LOG(ERROR) << "unload: " << dlerror();
#else
            FreeLibrary((HMODULE)(this->libOpenCL_));
            this->libOpenCL_ = NULL;
#endif
        }
    }
}

cl_int OpenCLAPI::ReleaseContextInternel_(cl_context ctx) {
    // cl_int rc = ppl::core::ocl::ReleaseAllKernelOfPool(ctx);
    // rc |= this->ReleaseContext_(ctx);
    // return rc;
    return 0;
}

}}} // namespace ppl::common::ocl
