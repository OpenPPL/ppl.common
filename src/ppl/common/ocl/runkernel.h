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

#ifndef _ST_HPC_PPL_COMMON_OCL_KERNEL_H_
#define _ST_HPC_PPL_COMMON_OCL_KERNEL_H_

#include "framechain.h"
#include "kernelpool.h"

#include <string>
#include <vector>

#include "ppl/common/log.h"

namespace ppl { namespace common { namespace ocl {

bool getKernelNames(const cl_program program, std::vector<std::string>& kernel_names);
bool buildProgram(const cl_program& program, const cl_device_id& device_id, const std::string& build_options);
bool compileOclKernels(FrameChain* frame_chain, const std::string& kernel_name);
bool validateNDrange(cl_uint work_dims, size_t* global_work_size, size_t* local_work_size);
bool enqueueOclKernel(FrameChain* frame_chain, const char* kernel_name, const cl_kernel& kernel, cl_uint work_dims,
                      const size_t* global_work_size, const size_t* local_work_size);
bool enqueueOclKernel_tunning(FrameChain* frame_chain, const char* kernel_name, const cl_kernel& kernel,
                              cl_uint work_dims, const size_t* global_work_size, const size_t* local_work_size);

template <size_t INDEX, typename T>
cl_int setKernelArg(const cl_kernel& kernel, const T& value) {
    cl_int error_code;
    error_code = clSetKernelArg(kernel, INDEX, sizeof(T), (void*)&value);
    if (error_code != CL_SUCCESS) {
        LOG(ERROR) << "Call clSetKernelArg() with the argument index " << INDEX << " failed with code: " << error_code;
    }

    return error_code;
}

template <size_t INDEX, typename T, typename... Args>
cl_int setKernelArg(const cl_kernel& kernel, const T& value, const Args&... rest) {
    cl_int error_code;
    error_code = clSetKernelArg(kernel, INDEX, sizeof(T), &value);
    if (error_code != CL_SUCCESS) {
        LOG(ERROR) << "Call clSetKernelArg() with the argument index " << INDEX << " failed with code: " << error_code;
        return error_code;
    }

    return setKernelArg<INDEX + 1, Args...>(kernel, rest...);
}

template <typename... Args>
void runOclKernel(FrameChain* frame_chain, const char* kernel_name_cstr, cl_uint work_dims, size_t* global_work_size,
                  size_t* local_work_size, Args... args) {
    cl_int error_code;
    bool succeeded;
    cl_context context = frame_chain->getContext();
    std::string project_name = frame_chain->getProjectName();
    std::string kernel_name = kernel_name_cstr;

#if 0
    // common used print tool when cross platform
    fprintf(stderr, "runOclKernel, kernel_name: %s\n", kernel_name_cstr);
    fprintf(stderr, "work_dims: %d, global_work_size: %d %d %d, local_work_size: %d %d %d\n",
            work_dims,
            global_work_size[0], (work_dims > 1) ? global_work_size[1] : 1, (work_dims > 2) ? global_work_size[2] : 1,
            local_work_size ? local_work_size[0] : 1,
            (local_work_size && (work_dims > 1)) ? local_work_size[1] : 1,
            (local_work_size && (work_dims > 2)) ? local_work_size[2] : 1);
#endif

    bool tuning = frame_chain->getTuningQueueStatus();
    frame_chain->setKernelTime(UINT64_MAX);

    cl_kernel kernel = getKernelFromPool(context, project_name, kernel_name + frame_chain->getCompileOptions());
    if (kernel == nullptr) {
        succeeded = compileOclKernels(frame_chain, kernel_name);
        if (!succeeded) {
            if (!tuning)
                LOG(ERROR) << "Failed to compile " << kernel_name;
            return;
        }
        cl_program program = frame_chain->getProgram();

        std::vector<std::string> kernel_names;
        getKernelNames(program, kernel_names);
        cl_kernel temp_kernel = nullptr;
        for (size_t i = 0; i < kernel_names.size(); i++) {
            temp_kernel = clCreateKernel(program, kernel_names[i].c_str(), &error_code);
            if (error_code != CL_SUCCESS) {
                if (!tuning)
                    LOG(ERROR) << "Call clCreateKernel() failed with code: " << error_code << ", " << kernel_name_cstr;
                return;
            }
            if (kernel_names[i] == kernel_name) {
                kernel = temp_kernel;
            }

            succeeded = insertKernelToPool(context, project_name, kernel_names[i] + frame_chain->getCompileOptions(),
                                           temp_kernel);
            if (!succeeded) {
                if (!tuning)
                    LOG(ERROR) << "Failed to insert kernel " << kernel_names[i] << " to kernel pool.";
            }
        }
        clReleaseProgram(program);
    }
    if (kernel == nullptr) {
        if (!tuning)
            LOG(ERROR) << "No valid kernel to run: " << kernel_name_cstr;
        return;
    }

    error_code = setKernelArg<0, Args...>(kernel, args...);
    if (error_code != CL_SUCCESS) {
        LOG(ERROR) << "setKernelArg failed: " << kernel_name_cstr;
        return;
    }

    succeeded = validateNDrange(work_dims, global_work_size, local_work_size);
    if (!succeeded) {
        if (!tuning)
            LOG(ERROR) << "Invalid NDrange of kernel " << kernel_name_cstr;
        return;
    }

    if (frame_chain->getTuningQueueStatus()) {
        succeeded = enqueueOclKernel_tunning(frame_chain, kernel_name_cstr, kernel, work_dims, global_work_size,
                                             local_work_size);
    } else {
        succeeded =
            enqueueOclKernel(frame_chain, kernel_name_cstr, kernel, work_dims, global_work_size, local_work_size);
    }
    if (!succeeded) {
        if (!tuning)
            LOG(ERROR) << "Failed to enqueue kernel " << kernel_name_cstr;
        return;
    }
}

}}} // namespace ppl::common::ocl

#endif
