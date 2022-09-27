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

namespace ppl { namespace common { namespace ocl {

bool compileOclKernels(FrameChain& frame_chain,
                       const char* build_options) {
    bool succeeded;
    succeeded = frame_chain.queryContext();
    if (!succeeded) return false;
    const char* strings = frame_chain.getCodeString();
    const size_t lengths = strlen(strings);

    cl_int error_code;
    cl_program program;
    program = clCreateProgramWithSource(frame_chain.getContext(), 1,
                                        &strings, &lengths, &error_code);
    if (error_code != CL_SUCCESS) {
        LOG(ERROR) << "Call clCreateProgramWithSource() failed with code: "
                   << error_code;
        return false;
    }
    frame_chain.setProgram(program);

    succeeded = frame_chain.queryDevice();
    if (!succeeded) return false;
    cl_device_id device_id = frame_chain.getDeviceId();

    error_code = clBuildProgram(program, 1, &device_id, build_options, nullptr,
                                 nullptr);
    if (error_code != CL_SUCCESS) {
        LOG(ERROR) << "Call clBuildProgram() failed with code: "
                   << error_code;
        return false;
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

bool enqueueOclKernel(FrameChain& frame_chain, const char* kernel_name,
                      const cl_kernel& kernel, cl_uint work_dims,
                      const size_t* global_work_size,
                      const size_t* local_work_size) {
    frame_chain.queryProfiling();
    bool profiling = frame_chain.isProfiling();

    cl_int error_code;
    cl_event event;
    if (profiling) {
        error_code = clEnqueueNDRangeKernel(frame_chain.getQueue(), kernel,
                         work_dims, nullptr, global_work_size, local_work_size,
                         0, nullptr, &event);
        if (error_code != CL_SUCCESS) {
            LOG(ERROR) << "Call clEnqueueNDRangeKernel() failed with code: "
                       << error_code;
            return false;
        }

        cl_ulong enqueue = 0;
        cl_ulong submit = 0;
        cl_ulong start = 0;
        cl_ulong end = 0;
        cl_ulong complete = 0;
        cl_ulong time = 0;

        PROFILE_INFO(CL_PROFILING_COMMAND_QUEUED, enqueue);
        PROFILE_INFO(CL_PROFILING_COMMAND_SUBMIT, submit);
        PROFILE_INFO(CL_PROFILING_COMMAND_START, start);
        PROFILE_INFO(CL_PROFILING_COMMAND_END, end);
        PROFILE_INFO(CL_PROFILING_COMMAND_COMPLETE, complete);

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
        error_code = clEnqueueNDRangeKernel(frame_chain.getQueue(), kernel,
                         work_dims, nullptr, global_work_size, local_work_size,
                         0, nullptr, nullptr);
        if (error_code != CL_SUCCESS) {
            LOG(ERROR) << "Call clEnqueueNDRangeKernel() failed with code: "
                    << error_code;
            return false;
        }
    }

    return true;
}

}}}
