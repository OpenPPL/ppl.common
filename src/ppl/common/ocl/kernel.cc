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

#include "kernel.h"

#include <string.h>
#include <string>

// #include <iostream>  // debug

#include "device.h"

namespace ppl { namespace common { namespace ocl {

#define ROUNDING0 32
#define ROUNDING1 4
#define SHIFT0 5
#define SHIFT1 2

void splitString(const std::string& str, const std::string& delimiter,
                 std::vector<std::string >& segments) {
    size_t last = 0;
    size_t index = str.find_first_of(delimiter, last);
    while (index != std::string::npos) {
        segments.push_back(str.substr(last, index - last));
        last = index + 1;
        index = str.find_first_of(delimiter, last);
    }

    if (str.length() - last > 0) {
        segments.push_back(str.substr(last));
    }
}

bool getKernelNames(const cl_program program,
                    std::vector<std::string>& kernel_names) {
    cl_int error_code;
    size_t returned_size;
    error_code = clGetProgramInfo(program, CL_PROGRAM_KERNEL_NAMES, 0, nullptr,
                                  &returned_size);
    if (error_code != CL_SUCCESS) {
        LOG(ERROR) << "Call clGetProgramInfo() failed with code: "
                   << error_code;
        return false;
    }

    if (returned_size <= 0) {
        LOG(ERROR) << "No kernel is detected by clGetProgramInfo().";
        return false;
    }

    std::string param_value;
    param_value.resize(returned_size - 1);
    error_code = clGetProgramInfo(program, CL_PROGRAM_KERNEL_NAMES, returned_size,
                                 (void*)param_value.data(), nullptr);
    if (error_code != CL_SUCCESS) {
        LOG(ERROR) << "Call clGetProgramInfo() failed with code: "
                   << error_code;
        return false;
    }

    // LOG(INFO) << "In getKernelNames, kernels name: " << param_value;
    // LOG(INFO) << "In getKernelNames, kernels size: " << param_value.size();
    // LOG(INFO) << "In getKernelNames, kernels size: " << returned_size;
    splitString(param_value, ";", kernel_names);

    return true;
}

bool compileOclKernels(FrameChain* frame_chain) {
    cl_context context = frame_chain->getContext();
    const char* source_str = frame_chain->getCodeString();
    const size_t kernel_length = strlen(source_str);

    cl_int error_code;
    cl_program program;
    program = clCreateProgramWithSource(context, 1, &source_str, &kernel_length,
                                        &error_code);
    if (error_code != CL_SUCCESS) {
        LOG(ERROR) << "Call clCreateProgramWithSource() failed with code: "
                   << error_code;
        return false;
    }
    frame_chain->setProgram(program);

    cl_device_id device_id = frame_chain->getDeviceId();
    std::string build_options = frame_chain->getCompileOptions();
    error_code = clBuildProgram(program, 1, &device_id, build_options.c_str(),
                                nullptr, nullptr);
    if (error_code != CL_SUCCESS) {
        size_t log_length;
        cl_int code = clGetProgramBuildInfo(program, device_id,
                          CL_PROGRAM_BUILD_LOG, 0, nullptr, &log_length);
        if (code != CL_SUCCESS) {
            LOG(ERROR) << "Call clGetProgramBuildInfo() failed with code: "
                       << code;
            return false;
        }

        std::string log_buffer((int)log_length, '0');
        code = clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG,
                                     log_length, (char*)log_buffer.data(),
                                     nullptr);
        if (code != CL_SUCCESS) {
            LOG(ERROR) << "Call clGetProgramBuildInfo() failed with code: "
                       << code;
            return false;
        }
        LOG(ERROR) << "Call clBuildProgram() failed with code: " << error_code;
        LOG(ERROR) << "clBuildProgram() log: " << log_buffer;

        return false;
    }

    return true;
}

bool validateNDrange(cl_uint work_dims, size_t* global_work_size,
                     size_t* local_work_size) {
    Device* device = getSharedDevice();
    size_t max_work_dim = device->getMaxWorkDims();
    size_t max_items_in_group = device->getMaxWorkItemsInGroup();
    std::vector<size_t> max_group_items_per_dim =
        device->getMaxItemsPerGroupDim();
    int opencl_version = device->getOpenCLVersion();

    if (work_dims < 1 || work_dims > max_work_dim) {
        LOG(ERROR) << "Invalid dimensions of work items: " << work_dims;
        return false;
    }

    if (global_work_size == nullptr) {
        LOG(ERROR) << "No valid global work size is passed.";
        return false;
    }

    cl_uint i = 0;
    for (; i < work_dims; i++) {
        if (global_work_size[i] == 0) {
            LOG(ERROR) << "Invalid number of global work item at dimension "
                       << i << ".";
            return false;
        }
    }

    if (local_work_size == nullptr) {
        if (opencl_version < 200) {
            global_work_size[0] = ((global_work_size[0] + ROUNDING0 - 1)
                                   >> SHIFT0) << SHIFT0;
            for (i = 1; i < work_dims; i++) {
                global_work_size[i] = ((global_work_size[i] + ROUNDING1 - 1)
                                       >> SHIFT1) << SHIFT1;
            }
        }
        return true;
    }

    size_t total_items = 1;
    for (i = 0; i < work_dims; i++) {
        total_items *= local_work_size[i];
    }
    if (total_items > max_items_in_group) {
        LOG(ERROR) << "Total number of work items in a group must less than or "
                   << "equal to : " << max_items_in_group;
        return false;
    }

    for (i = 0; i < work_dims; i++) {
        if (local_work_size[i] > max_group_items_per_dim[i]) {
            LOG(ERROR) << "Number of work items in group[" << i
                       << "] must less than or equal to : "
                       << max_group_items_per_dim[i];
            return false;
        }
    }

    if (opencl_version < 200 || device->getGpuType() == MALI_GPU) {
        for (i = 0; i < work_dims; i++) {
            global_work_size[i] = ((global_work_size[i] + local_work_size[i] -
                                   1) / local_work_size[i]) *
                                   local_work_size[i];
        }
    }

    return true;
}

#define PROFILE_INFO(status, value)                                            \
    error_code = clGetEventProfilingInfo(event, status, sizeof(cl_ulong),      \
                                         &value, nullptr);                     \
    if (error_code != CL_SUCCESS) {                                            \
        LOG(ERROR) << "Call clGetEventProfilingInfo() failed with code: "      \
                   << error_code;                                              \
    }

bool enqueueOclKernel(FrameChain* frame_chain, const char* kernel_name,
                      const cl_kernel& kernel, cl_uint work_dims,
                      const size_t* global_work_size,
                      const size_t* local_work_size) {
    bool profiling = frame_chain->isProfiling();

    cl_int error_code;
    if (profiling) {
        cl_event event;
        error_code = clEnqueueNDRangeKernel(frame_chain->getQueue(), kernel,
                         work_dims, nullptr, global_work_size, local_work_size,
                         0, nullptr, &event);
        if (error_code != CL_SUCCESS) {
            LOG(ERROR) << "Call clEnqueueNDRangeKernel() failed with code: "
                       << error_code;
            return false;
        }

        error_code = clWaitForEvents(1, &event);
        if (error_code != CL_SUCCESS) {
            LOG(ERROR) << "Call clWaitForEvents() failed with code: "
                       << error_code;
            return false;
        }

        cl_ulong enqueue = 0;
        cl_ulong submit = 0;
        cl_ulong start = 0;
        cl_ulong end = 0;
#if CL_TARGET_OPENCL_VERSION >= 200
        cl_ulong complete = 0;
#endif
        cl_ulong time = 0;

        PROFILE_INFO(CL_PROFILING_COMMAND_QUEUED, enqueue);
        PROFILE_INFO(CL_PROFILING_COMMAND_SUBMIT, submit);
        PROFILE_INFO(CL_PROFILING_COMMAND_START, start);
        PROFILE_INFO(CL_PROFILING_COMMAND_END, end);
#if CL_TARGET_OPENCL_VERSION >= 200
        PROFILE_INFO(CL_PROFILING_COMMAND_COMPLETE, complete);
#endif
        time = end - start;
        LOG(INFO) << "Execution time of " << kernel_name << ": " << time
                  << " ns.";

        error_code = clReleaseEvent(event);
        if (error_code != CL_SUCCESS) {
            LOG(ERROR) << "Call clReleaseEvent() failed with code: "
                       << error_code;
            return false;
        }
    }
    else {
        error_code = clEnqueueNDRangeKernel(frame_chain->getQueue(), kernel,
                         work_dims, nullptr, global_work_size, local_work_size,
                         0, nullptr, nullptr);
        if (error_code != CL_SUCCESS) {
            LOG(ERROR) << "Call clEnqueueNDRangeKernel() failed with code: "
                       << error_code;
            return false;
        }
    }
    // clFinish(frame_chain->getQueue());
    // clFlush(frame_chain->getQueue());

    return true;
}

}}}
