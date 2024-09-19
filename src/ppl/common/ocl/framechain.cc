// Licensed to the Apache Software Foundation (ASF) under one
// or more contributor license agreements.  See the NOTICE file
// distributed with this work for additional information
// regarding copyright ownership.  The ASF licenses this file
// to you under the Apache License, Version 2.0 (the
// "License"); you may not use this file except in compliance
// with the License.  You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing,
// software distributed under the License is distributed on an
// "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied.  See the License for the
// specific language governing permissions and limitations
// under the License.

#include "framechain.h"
#include "device.h"

#include <vector>
#include <string.h>

#include "ppl/common/log.h"

//    cl_qcom_perf_hint extension
typedef cl_uint cl_perf_hint;
#define CL_CONTEXT_PERF_HINT_QCOM   0x40C2

//   cl_perf_hint
#define CL_PERF_HINT_HIGH_QCOM      0x40C3
#define CL_PERF_HINT_NORMAL_QCOM    0x40C4
#define CL_PERF_HINT_LOW_QCOM       0x40C5

//    cl_qcom_priority_hint extension
typedef cl_uint cl_priority_hint;
#define CL_PRIORITY_HINT_NONE_QCOM 0
#define CL_CONTEXT_PRIORITY_HINT_QCOM   0x40C9

//    cl_priority_hint
#define CL_PRIORITY_HINT_HIGH_QCOM      0x40CA
#define CL_PRIORITY_HINT_NORMAL_QCOM    0x40CB
#define CL_PRIORITY_HINT_LOW_QCOM       0x40CC

namespace ppl { namespace common { namespace ocl {

#define MAX_EXT_CHAR_LENGTH (1024 * 16)

// qualcomm 5xx when param_value is null need param_value_size >= sizeof(param_name);
#define OCLINFOAPI_IMP(api, obj, info)                                    \
    do {                                                                  \
        cl_int rc = 0;                                                    \
        size_t s = 0;                                                     \
                                                                          \
        rc = api(obj, info, 1024, nullptr, &s);                           \
        if (CL_SUCCESS != rc) {                                           \
            LOG(ERROR) << "Call " << #api << " failed with code: " << rc; \
            return std::vector<uint8_t>();                                \
        }                                                                 \
        std::vector<uint8_t> value(s);                                    \
        rc = api(obj, info, s, (void*)value.data(), nullptr);             \
        if (CL_SUCCESS != rc) {                                           \
            LOG(ERROR) << "Call " << #api << " failed with code: " << rc; \
            return std::vector<uint8_t>();                                \
        }                                                                 \
        return value;                                                     \
    } while (0)

#define INFO(api, object_type, arg)              \
    api(object_type arg, cl_##arg##_info info) { \
        OCLINFOAPI_IMP(cl##api, arg, info);      \
        return std::vector<uint8_t>();           \
    }

std::vector<uint8_t> INFO(GetPlatformInfo, cl_platform_id, platform) std::vector<uint8_t> INFO(
    GetDeviceInfo, cl_device_id, device) std::vector<uint8_t> INFO(GetContextInfo, cl_context, context)
    std::vector<uint8_t> INFO(GetCommandQueueInfo, cl_command_queue, command_queue) std::vector<uint8_t> INFO(
        GetKernelInfo, cl_kernel, kernel) std::vector<uint8_t> INFO(GetProgramInfo, cl_program, program)
        std::vector<uint8_t> INFO(GetImageInfo, cl_mem, image) std::vector<uint8_t> INFO(
            GetMemObjectInfo, cl_mem, mem) std::vector<uint8_t> INFO(GetEventProfilingInfo, cl_event, profiling)
#undef INFO
#undef OCLINFOAPI_IMP

            std::string GetDeviceDesc(cl_device_id device) {
    std::string dev_desc;

    std::vector<uint8_t> vendor = GetDeviceInfo(device, CL_DEVICE_VENDOR);
    std::string dev_vendor((char*)vendor.data());
    char d[64] = "";
    if (dev_vendor == "ARM") {
        std::vector<uint8_t> tmp = GetDeviceInfo(device, CL_DEVICE_NAME);
        d[0] = tmp[5];
        d[1] = tmp[6];
        d[2] = tmp[7];
        d[3] = '\0';
    } else if (dev_vendor == "QUALCOMM") {
        std::vector<uint8_t> tmp = GetDeviceInfo(device, CL_DEVICE_VERSION);
        std::string version_(tmp.begin(), tmp.end());
        int nPos = version_.find("Adreno");
        version_ = version_.substr(nPos);
        d[0] = '0';
        int j = 0;
        for (int i = 0; i < version_.length(); i++) {
            if ((version_[i] >= '0' && version_[i] <= '9') || (version_[i] >= 'a' && version_[i] <= 'z') ||
                (version_[i] >= 'A' && version_[i] <= 'Z')) {
                d[j++] = version_[i];
            }
        }
        d[j] = '\0';
    } else {
        LOG(WARNING) << "unopt device: " << dev_vendor;
    }

    dev_desc += std::string(d);

    return (dev_vendor + std::string("_") + dev_desc);
}

FrameChain::FrameChain(bool profiling, int perf_hint, int priority_hint)
    : platform_id_(nullptr)
    , device_id_(nullptr)
    , context_(nullptr)
    , queue_(nullptr)
    , tuning_queue_(nullptr)
    , device_desc_("")
    , vendor_desc_("")
    , program_(nullptr)
    , creating_program_type_(WITH_SOURCE)
    , source_file_name_(nullptr)
    , source_string_(nullptr)
    , profiling_(false)
    , save_program_binary_(false)
    , opt_level_(0)
    , tuning_queue_on_(false) {
    createDefaultOclFrame(profiling, perf_hint, priority_hint);
}

FrameChain::FrameChain(const cl_command_queue& queue)
    : platform_id_(nullptr)
    , device_id_(nullptr)
    , context_(nullptr)
    , tuning_queue_(nullptr)
    , device_desc_("")
    , vendor_desc_("")
    , program_(nullptr)
    , creating_program_type_(WITH_SOURCE)
    , source_file_name_(nullptr)
    , source_string_(nullptr)
    , profiling_(false)
    , save_program_binary_(false)
    , opt_level_(0)
    , tuning_queue_on_(false) {
    if (queue == nullptr) {
        LOG(ERROR) << "Invalid command queue.";
        return;
    }

    queue_ = queue;
    queryProfiling();

    cl_int error_code;
    error_code = clGetCommandQueueInfo(queue_, CL_QUEUE_CONTEXT, sizeof(cl_context), &context_, nullptr);
    if (error_code != CL_SUCCESS) {
        LOG(ERROR) << "Call clGetCommandQueueInfo failed with code: " << error_code;
        return;
    }

    error_code = clGetCommandQueueInfo(queue_, CL_QUEUE_DEVICE, sizeof(cl_device_id), &device_id_, nullptr);
    if (error_code != CL_SUCCESS) {
        LOG(ERROR) << "Call clGetCommandQueueInfo failed with code: " << error_code;
        return;
    }

    error_code = clGetDeviceInfo(device_id_, CL_DEVICE_PLATFORM, sizeof(cl_platform_id), &platform_id_, nullptr);
    if (error_code != CL_SUCCESS) {
        LOG(ERROR) << "Call clGetDeviceInfo() failed with code: " << error_code;
        return;
    }

    device_desc_ = GetDeviceDesc(device_id_);

    std::vector<uint8_t> vendor = GetDeviceInfo(device_id_, CL_DEVICE_VENDOR);
    std::string dev_vendor((char*)vendor.data());
    vendor_desc_ = dev_vendor;

    get_extention_info();
}

FrameChain::~FrameChain() {
    if (this->isProfiling()) {
        releaseEventList();
    }
    if (this->queue_) {
        clReleaseCommandQueue(this->queue_);
    }
    if (this->tuning_queue_) {
        clReleaseCommandQueue(this->tuning_queue_);
    }
    if (this->context_) {
        clReleaseContext(this->context_);
    }
}

void FrameChain::printEventList() {
    for (int i = 0; i < this->event_list.size(); i++) {
        cl_int status;
        cl_ulong start = 0;
        cl_ulong end = 0;
        cl_ulong time = 0;
        clGetEventInfo(this->event_list[i].event, CL_EVENT_COMMAND_EXECUTION_STATUS, sizeof(cl_int), &status, nullptr);
        if (status == CL_COMPLETE) {
            clGetEventProfilingInfo(this->event_list[i].event, CL_PROFILING_COMMAND_START, sizeof(cl_ulong), &start,
                                    NULL);
            clGetEventProfilingInfo(this->event_list[i].event, CL_PROFILING_COMMAND_END, sizeof(cl_ulong), &end, NULL);
            time = end - start;
            LOG(INFO) << "Execution time of " << this->event_list[i].kernel_name << ": " << time << " ns.";
        } else {
            LOG(ERROR) << "clGetEventProfilingInfo status not equal CL_COMPLETE ";
        }
    }
}

void FrameChain::releaseEventList() {
    for (int i = 0; i < this->event_list.size(); i++) {
        if (this->event_list[i].event)
            clReleaseEvent(this->event_list[i].event);
    }
    this->event_list.clear();
}

void FrameChain::setProgram(const cl_program program) {
    if (program == nullptr) {
        LOG(ERROR) << "Invalid OpenCL program.";
    }

    program_ = program;
}

void FrameChain::setCreatingProgramType(const CreatingProgramTypes creating_program_type) {
    creating_program_type_ = creating_program_type;
}

void FrameChain::setSourceFileName(const char* source_file_name) {
    if (source_file_name == nullptr) {
        LOG(ERROR) << "Invalid address of the source file name.";
    }

    source_file_name_ = (char*)source_file_name;
}

void FrameChain::setSaveProgramBinaryFlag(bool save_program_binary) {
    save_program_binary_ = save_program_binary;
}

void FrameChain::setOptLevel(uint32_t opt_level) {
    opt_level_ = opt_level;
}

void FrameChain::setSource(const char* source_string) {
    if (source_string == nullptr) {
        LOG(ERROR) << "Invalid address of the source string.";
    }

    source_string_ = (char*)source_string;
}

void FrameChain::setSpir(const void* spir_string, size_t spir_size) {
    if (spir_string == nullptr) {
        LOG(ERROR) << "Invalid address of the spir string.";
    }

    if (spir_size == 0) {
        LOG(ERROR) << "Invalid size of the spir string.";
    }

    spir_string_ = (void*)spir_string;
    spir_size_ = spir_size;
}

void FrameChain::setProjectName(const char* project_name) {
    if (project_name == nullptr) {
        LOG(ERROR) << "Invalid address of the project name.";
    }

    project_name_ = project_name;
}

void FrameChain::setFunctionName(const char* function_name) {
    if (function_name == nullptr) {
        LOG(ERROR) << "Invalid address of the function name.";
    }

    function_name_ = function_name;
}

void FrameChain::setCompileOptions(const char* options) {
    compile_options_ = options;
    compile_options_ += compile_options_ext_defaults_ ;
}

void arm_printf_callback(const char* buffer, size_t length, size_t final, void* user_data) {
    fwrite(buffer, 1, length, stdout);
}

bool FrameChain::ifSupportQcomHints(){
    char ext_info_str[MAX_EXT_CHAR_LENGTH];
    cl_int err = clGetDeviceInfo(device_id_, CL_DEVICE_EXTENSIONS, MAX_EXT_CHAR_LENGTH, ext_info_str, nullptr);
    if (err != CL_SUCCESS) {
        LOG(ERROR) << " Invalid clGetDeviceInfo ! ";
    }

    bool perf_flag = false;
    if (strstr(ext_info_str, "cl_qcom_perf_hint") != NULL) {
        perf_flag = true;
    }
    bool priority_flag = false;
    if (strstr(ext_info_str, "cl_qcom_priority_hint") != NULL) {
        priority_flag = true;
    }

    return (perf_flag && priority_flag);
}

void FrameChain::get_extention_info() {
    char ext_info_str[MAX_EXT_CHAR_LENGTH];

    compile_options_ext_defaults_ = " ";

    cl_int err = clGetDeviceInfo(device_id_, CL_DEVICE_EXTENSIONS, MAX_EXT_CHAR_LENGTH, ext_info_str, nullptr);
    if (err != CL_SUCCESS) {
        LOG(ERROR) << " Invalid clGetDeviceInfo ! ";
    }
  
    if (strstr(ext_info_str, "cl_khr_subgroups") != NULL) {
        is_support_subgroup = true;
    }

    if (strstr(ext_info_str, "cl_khr_fp16") != NULL) {
        is_support_fp16 = true;
    }

    if (strstr(ext_info_str, "cl_khr_3d_image_writes") != NULL) {
        is_support_3d_image_write = true;
    }

    // speed up the vendor conditions
    if (vendor_desc_ == "QUALCOMM")
    {
        compile_options_ext_defaults_ += " -DVENDOR_QUALCOMM";
        platform_type0 = PlatformType0_QCOM;
    }
    else if (vendor_desc_ == "ARM")
    {
        platform_type0 = PlatformType0_ARM;
        compile_options_ext_defaults_ += " -DVENDOR_ARM";
    }
        
    else  if (strstr(vendor_desc_.c_str(), "Intel") != NULL)
    {
        platform_type0 = PlatformType0_INTEL;
        compile_options_ext_defaults_ += " -DVENDOR_INTEL";

    }
        
    else 
    {
        platform_type0 = PlatformType0_invalid;
        compile_options_ext_defaults_ += " -DVENDOR_UNKNOW";
    }
        

    //no need for cl_khr_integer_dot_product
    //if (strstr(ext_info_str, "cl_khr_integer_dot_product") != NULL) 
    {
        // qcom
        if (platform_type0 == PlatformType0_QCOM) {
            if (strstr(ext_info_str, "cl_qcom_dot_product8") != NULL) {
                is_support_int8_product = true;
            }
        }
        else if  (platform_type0 == PlatformType0_ARM){
            if (strstr(ext_info_str, "cl_arm_integer_dot_product_accumulate_int8") != NULL) {
                is_support_int8_product = true;
            }
        }
        else{
            //todo , not exactly 
            if (strstr(ext_info_str, "cl_khr_integer_dot_product") != NULL) {
                is_support_int8_product = true;
            }
        }
        // others todo

    }

    //shuffle and rotate

    if (platform_type0 == PlatformType0_QCOM) {
        if (strstr(ext_info_str, "cl_qcom_reqd_sub_group_size") != NULL) {
            getQcomExtInfo()->is_support_reqd_sub_group_size = true;

    
        }

        if (strstr(ext_info_str, "cl_qcom_subgroup_shuffle") != NULL) {
            getQcomExtInfo()->is_support_subgroup_shuffle = true;

            is_support_subgroup_shuffle = true;
            is_support_subgroup_rotate = true;

            compile_options_ext_defaults_ += " -DSUBGROUP_SHUFFLE_ENABLED ";
            compile_options_ext_defaults_ += " -DSUBGROUP_ROTATE_ENABLED ";

        }
    }
    else if (platform_type0 == PlatformType0_INTEL) {
        if (strstr(ext_info_str, "cl_intel_subgroups") != NULL) {
            is_support_subgroup_shuffle = true;
            is_support_subgroup_rotate = true;

            getIntelExtInfo()->is_support_intel_enhanced_shuffle = true;

            compile_options_ext_defaults_ += " -DSUBGROUP_SHUFFLE_ENABLED ";
            compile_options_ext_defaults_ += " -DSUBGROUP_ROTATE_ENABLED ";
            compile_options_ext_defaults_ += " -DINTEL_SUBGROUP_ENHANCED ";

        }
        else{
            if (strstr(ext_info_str, "cl_khr_subgroup_shuffle") != NULL) {
                is_support_subgroup_shuffle = true;
                compile_options_ext_defaults_ += " -DSUBGROUP_SHUFFLE_ENABLED ";
            }

            if (strstr(ext_info_str, "cl_khr_subgroup_rotate") != NULL) {
                is_support_subgroup_rotate = true;
                compile_options_ext_defaults_ += " -DSUBGROUP_ROTATE_ENABLED ";
            }
        }

    }
    else {

        //arm like
        if (strstr(ext_info_str, "cl_khr_subgroup_shuffle") != NULL) {
            is_support_subgroup_shuffle = true;
            compile_options_ext_defaults_ += " -DSUBGROUP_SHUFFLE_ENABLED ";
        }

        if (strstr(ext_info_str, "cl_khr_subgroup_rotate") != NULL) {
            is_support_subgroup_rotate = true;
            compile_options_ext_defaults_ += " -DSUBGROUP_ROTATE_ENABLED ";
        }
    }


    if(platform_type0 == PlatformType0_QCOM || platform_type0 == PlatformType0_ARM) {

        // get max sub group size
        const char* kernelSource = "__kernel void exampleKernel() {}; ";
        size_t kernel_length = strlen(kernelSource);
        cl_program program = clCreateProgramWithSource(context_, 1, &kernelSource, &kernel_length, &err);
        if (err != CL_SUCCESS) {
            LOG(ERROR) << " clCreateProgramWithSource failed when get subgroup size ! ";
            clReleaseProgram(program);
            return;
        }

        err = clBuildProgram(program, 1, &(device_id_), NULL, NULL, NULL);
        if (err != CL_SUCCESS) {
            LOG(ERROR) << " Invalid clBuildProgram when get subgroup size ! ";
            clReleaseProgram(program);
            return;
        }

        cl_kernel kernel = clCreateKernel(program, "exampleKernel", &err);
        if (err != CL_SUCCESS) {
            LOG(ERROR) << " Invalid clCreateKernel when get subgroup size ! ";
            clReleaseKernel(kernel);
            clReleaseProgram(program);

            return;
        }

        err = clGetKernelSubGroupInfoKHR(kernel,
                                         device_id_,
                                         CL_KERNEL_MAX_SUB_GROUP_SIZE_FOR_NDRANGE,
                                         0,
                                         (const void*)NULL,
                                         sizeof(max_subgroup_size_),
                                         &max_subgroup_size_,
                                         NULL);

        clReleaseKernel(kernel);
        clReleaseProgram(program);
        
        if (err != CL_SUCCESS) {
            LOG(ERROR) << " Invalid clGetKernelSubGroupInfo when get subgroup size ! ";
            return;
        }
    }
    else //if(platform_type0 == PlatformType0_INTEL) //todo
    {
        auto context = clCreateContext(NULL, 1, &device_id_, NULL, NULL, &err);
        auto command_queue = clCreateCommandQueue(context, device_id_, 0, &err);
        auto buffer = clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(float), NULL, &err);
        const char* kernel_source = "__kernel void example_kernel(__global int *buffer) {\
            int max_sub_group_size = get_max_sub_group_size();\
            if (get_global_id(0) == 0) {\
                buffer[0] = max_sub_group_size;\
            }\
        }";

        auto program = clCreateProgramWithSource(context, 1, &kernel_source, NULL, &err);
        err = clBuildProgram(program, 1, &device_id_, NULL, NULL, NULL);

        if (err != CL_SUCCESS) {
            LOG(ERROR) << " clBuildProgram failed when get subgroup size ! ";
            clReleaseProgram(program);
            clReleaseMemObject(buffer);
            clReleaseCommandQueue(command_queue);
            clReleaseContext(context);

            return;
        }

        cl_kernel kernel = clCreateKernel(program, "example_kernel", &err);
        err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &buffer);

        if (err != CL_SUCCESS) {
            LOG(ERROR) << " clSetKernelArg failed when get subgroup size ! ";
            clReleaseProgram(program);
            clReleaseKernel(kernel);
            clReleaseMemObject(buffer);
            clReleaseCommandQueue(command_queue);
            clReleaseContext(context);
            return;
        }

        size_t global_work_size = 1; 
        err = clEnqueueNDRangeKernel(command_queue, kernel, 1, NULL, &global_work_size, NULL, 0, NULL, NULL);
 
        if (err != CL_SUCCESS) {
            LOG(ERROR) << " clEnqueueNDRangeKernel failed when get subgroup size ! ";
            clReleaseProgram(program);
            clReleaseKernel(kernel);
            clReleaseMemObject(buffer);
            clReleaseCommandQueue(command_queue);
            clReleaseContext(context);
            return;
        }

        int max_sub_group_size;
        err = clEnqueueReadBuffer(command_queue, buffer, CL_TRUE, 0, sizeof(int), &max_sub_group_size, 0, NULL, NULL);
        if (err != CL_SUCCESS) {
            LOG(ERROR) << " clEnqueueReadBuffer failed when get subgroup size ! ";
            clReleaseProgram(program);
            clReleaseKernel(kernel);
            clReleaseMemObject(buffer);
            clReleaseCommandQueue(command_queue);
            clReleaseContext(context);
            return;
        }
        
        //printf("Max Sub Group Size: %u\n", (unsigned int)max_sub_group_size);
        max_subgroup_size_ = max_sub_group_size;

        clReleaseKernel(kernel);
        clReleaseProgram(program);
        clReleaseMemObject(buffer);
        clReleaseCommandQueue(command_queue);
        clReleaseContext(context);
        
    }

    // info if needed to print 
    
    //LOG(INFO) << " ext info: pl "<<(int)platform_type0<<" maxsbg: "<<max_subgroup_size_;
    //LOG(INFO) << " ext info: fp16 "<<is_support_fp16<<" sbg: "<<is_support_subgroup<<" 3d write:"
    //<<is_support_3d_image_write<<" int8 dot"<<is_support_int8_product<<" shuffle: "<<is_support_subgroup_shuffle
    //<<" rotate "<<is_support_subgroup_rotate;

    return ;
}

bool FrameChain::createDefaultOclFrame(bool profiling, int perf_hint, int priority_hint) {
    cl_int error_code;
    createSharedDevice();
    Device* device = getSharedDevice();
    platform_id_ = device->getPlatformId();
    device_id_ = device->getDeviceId();

    device_desc_ = GetDeviceDesc(device_id_);
    std::vector<uint8_t> vendor = GetDeviceInfo(device_id_, CL_DEVICE_VENDOR);
    std::string dev_vendor((char*)vendor.data());
    vendor_desc_ = dev_vendor;

    std::vector<cl_context_properties> context_properties;
    if (vendor_desc_ == "ARM") {
        // Initializing the printf functionality for ARM GPU
        context_properties.resize(7);
        context_properties[0] = CL_CONTEXT_PLATFORM;
        context_properties[1] = (cl_context_properties)device->getPlatformId();
        context_properties[2] = CL_PRINTF_CALLBACK_ARM;
        context_properties[3] = (cl_context_properties)arm_printf_callback;
        context_properties[4] = CL_PRINTF_BUFFERSIZE_ARM;
        context_properties[5] = 0X1000;
        context_properties[6] = 0;
    } else if (vendor_desc_ == "QUALCOMM" && ifSupportQcomHints()) {
        if (perf_hint != 0 && perf_hint != -1 && perf_hint != 1) {
            LOG(ERROR) << "Invalid perf hint value.";
        }
        if (priority_hint != 0 && priority_hint != -1 && priority_hint != 1) {
            LOG(ERROR) << "Invalid priority hint value.";
        }
        context_properties.resize(7);
        context_properties[0] = CL_CONTEXT_PERF_HINT_QCOM;
        context_properties[1] = (perf_hint == 0 ? CL_PERF_HINT_NORMAL_QCOM :
                (perf_hint == 1 ? CL_PERF_HINT_HIGH_QCOM : CL_PERF_HINT_LOW_QCOM));
        context_properties[2] = CL_CONTEXT_PRIORITY_HINT_QCOM;
        context_properties[3] = (priority_hint == 0 ? CL_PRIORITY_HINT_NORMAL_QCOM :
                (priority_hint == 1 ? CL_PRIORITY_HINT_HIGH_QCOM : CL_PRIORITY_HINT_LOW_QCOM));
        context_properties[4] = CL_CONTEXT_PLATFORM;
        context_properties[5] = (cl_context_properties)device->getPlatformId();
        context_properties[6] = 0;
    } else {
        context_properties.resize(3);
        context_properties[0] = CL_CONTEXT_PLATFORM;
        context_properties[1] = (cl_context_properties)device->getPlatformId();
        context_properties[2] = 0;
    }
    context_ = clCreateContext(context_properties.data(), 1, &device_id_, nullptr, nullptr, &error_code);
    if (error_code != CL_SUCCESS) {
        LOG(ERROR) << "Call clCreateContext failed with code: " << error_code;
        return false;
    }

    get_extention_info();

    profiling_ = profiling;
#if CL_TARGET_OPENCL_VERSION < 200
    cl_command_queue_properties queue_properties;
    if (profiling) {
        queue_properties = CL_QUEUE_PROFILING_ENABLE;
    } else {
        queue_properties = 0;
    }
    queue_ = clCreateCommandQueue(context_, device_id_, queue_properties, &error_code);
    if (error_code != CL_SUCCESS) {
        LOG(ERROR) << "Call clCreateCommandQueue failed with code: " << error_code;
        return false;
    } else {
        return true;
    }
#else
    std::vector<cl_queue_properties> queue_properties;
    queue_properties.resize(3);
    queue_properties[0] = CL_QUEUE_PROPERTIES;
    if (profiling) {
        queue_properties[1] = (cl_queue_properties)CL_QUEUE_PROFILING_ENABLE;
    } else {
        queue_properties[1] = (cl_queue_properties)0;
    }
    queue_properties[2] = 0;
    queue_ = clCreateCommandQueueWithProperties(context_, device_id_, queue_properties.data(), &error_code);
    if (error_code != CL_SUCCESS) {
        LOG(ERROR) << "Call clCreateCommandQueueWithProperties failed "
                   << "with code: " << error_code;
        return false;
    } else {
        return true;
    }
#endif
}

bool FrameChain::queryProfiling() {
    if (queue_ == nullptr) {
        LOG(ERROR) << "The command queue is uninitialized.";
        return false;
    }

    cl_int error_code;
    std::vector<cl_command_queue_properties> queue_properties(5);
    error_code = clGetCommandQueueInfo(queue_, CL_QUEUE_PROPERTIES, sizeof(cl_command_queue_properties),
                                       (void*)queue_properties.data(), nullptr);
    if (error_code != CL_SUCCESS) {
        LOG(ERROR) << "Call clGetCommandQueueInfo failed with code: " << error_code;
        return false;
    }

    Device* device = getSharedDevice();
    int opencl_version = device->getOpenCLVersion();
    if (opencl_version < 200) {
        if (queue_properties[0] == CL_QUEUE_PROFILING_ENABLE) {
            profiling_ = true;
        } else {
            profiling_ = false;
        }
    } else {
        if ((queue_properties[1] & CL_QUEUE_PROFILING_ENABLE) || (queue_properties[3] & CL_QUEUE_PROFILING_ENABLE)) {
            profiling_ = true;
        } else {
            profiling_ = false;
        }
    }

    return true;
}

cl_command_queue FrameChain::getTuningQueue() {
    cl_int error_code;
    if (!tuning_queue_) {
#if CL_TARGET_OPENCL_VERSION < 200
        cl_command_queue_properties queue_properties = CL_QUEUE_PROFILING_ENABLE;
        tuning_queue_ = clCreateCommandQueue(context_, device_id_, queue_properties, &error_code);
        if (error_code != CL_SUCCESS) {
            LOG(ERROR) << "Call clCreateCommandQueue failed with code: " << error_code;
            return nullptr;
        }
#else
        std::vector<cl_queue_properties> queue_properties;
        queue_properties.resize(3);
        queue_properties[0] = CL_QUEUE_PROPERTIES;
        queue_properties[1] = (cl_queue_properties)CL_QUEUE_PROFILING_ENABLE;

        queue_properties[2] = 0;
        tuning_queue_ = clCreateCommandQueueWithProperties(context_, device_id_, queue_properties.data(), &error_code);
        if (error_code != CL_SUCCESS) {
            LOG(ERROR) << "Call clCreateCommandQueueWithProperties failed "
                       << "with code: " << error_code;
            return nullptr;
        }
#endif
    }

    return tuning_queue_;
}

static FrameChain* shared_frame_chain;

void createSharedFrameChain(bool profiling, int perf_hint, int priority_hint) {
    static FrameChain frame_chain(profiling, perf_hint, priority_hint);

    shared_frame_chain = &frame_chain;
}

void createSharedFrameChain(const cl_command_queue& queue) {
    static FrameChain frame_chain(queue);

    shared_frame_chain = &frame_chain;
}

FrameChain* getSharedFrameChain() {
    return shared_frame_chain;
}

}}} // namespace ppl::common::ocl
