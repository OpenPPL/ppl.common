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

#include "ppl/common/log.h"

namespace ppl { namespace common { namespace ocl {

FrameChain::FrameChain(bool profiling) : platform_id_(nullptr),
        device_id_(nullptr), context_(nullptr), queue_(nullptr),
        program_(nullptr), creating_program_type_(WITH_SOURCE),
        source_string_(nullptr), profiling_(false) {
    createDefaultOclFrame(profiling);
}

FrameChain::FrameChain(const cl_command_queue& queue) : platform_id_(nullptr),
        device_id_(nullptr), context_(nullptr), program_(nullptr),
        creating_program_type_(WITH_SOURCE), source_string_(nullptr),
        profiling_(false) {
    if (queue == nullptr) {
        LOG(ERROR) << "Invalid command queue.";
        return;
    }

    queue_ = queue;
    queryProfiling();

    cl_int error_code;
    error_code = clGetCommandQueueInfo(queue_, CL_QUEUE_CONTEXT,
                                       sizeof(cl_context), &context_,
                                       nullptr);
    if (error_code != CL_SUCCESS) {
        LOG(ERROR) << "Call clGetCommandQueueInfo failed with code: "
                   << error_code;
        return;
    }

    error_code = clGetCommandQueueInfo(queue_, CL_QUEUE_DEVICE,
                                       sizeof(cl_device_id), &device_id_,
                                       nullptr);
    if (error_code != CL_SUCCESS) {
        LOG(ERROR) << "Call clGetCommandQueueInfo failed with code: "
                   << error_code;
        return;
    }

    error_code = clGetDeviceInfo(device_id_, CL_DEVICE_PLATFORM,
                                 sizeof(cl_platform_id), &platform_id_,
                                 nullptr);
    if (error_code != CL_SUCCESS) {
        LOG(ERROR) << "Call clGetDeviceInfo() failed with code: "
                   << error_code;
        return;
    }
}

FrameChain::~FrameChain() {
    if(this->queue_){
        clReleaseCommandQueue(this->queue_);
    }
    if(this->context_){
        clReleaseContext(this->context_);
    }
}

void FrameChain::setProgram(const cl_program program) {
    if (program == nullptr) {
        LOG(ERROR) << "Invalid OpenCL program.";
    }

    program_ = program;
}

void FrameChain::setCreatingProgramType(const CreatingProgramTypes
                                        creating_program_type) {
    creating_program_type_ = creating_program_type;
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
}

bool FrameChain::createDefaultOclFrame(bool profiling) {
    cl_int error_code;
    createSharedDevice();
    Device* device = getSharedDevice();
    platform_id_ = device->getPlatformId();
    device_id_ = device->getDeviceId();

    std::vector<cl_context_properties> context_properties;
    context_properties.resize(3);
    context_properties[0] = CL_CONTEXT_PLATFORM;
    context_properties[1] =
            (cl_context_properties)device->getPlatformId();
    context_properties[2] = 0;
    context_ = clCreateContext(context_properties.data(), 1, &device_id_,
                               nullptr, nullptr, &error_code);
    if (error_code != CL_SUCCESS) {
        LOG(ERROR) << "Call clCreateContext failed with code: " << error_code;
        return false;
    }

    profiling_ = profiling;
#if CL_TARGET_OPENCL_VERSION < 200
    cl_command_queue_properties queue_properties;
    if (profiling) {
        queue_properties = CL_QUEUE_PROFILING_ENABLE;
    }
    else {
        queue_properties = 0;
    }
    queue_ = clCreateCommandQueue(context_, device_id_, queue_properties,
                                  &error_code);
    if (error_code != CL_SUCCESS) {
        LOG(ERROR) << "Call clCreateCommandQueue failed with code: "
                    << error_code;
        return false;
    }
    else {
        return true;
    }
#else
    std::vector<cl_queue_properties> queue_properties;
    queue_properties.resize(3);
    queue_properties[0] = CL_QUEUE_PROPERTIES;
    if (profiling) {
        queue_properties[1] =
            (cl_queue_properties)CL_QUEUE_PROFILING_ENABLE;
    }
    else {
        queue_properties[1] = (cl_queue_properties)0;
    }
    queue_properties[2] = 0;
    queue_ = clCreateCommandQueueWithProperties(context_, device_id_,
                    queue_properties.data(), &error_code);
    if (error_code != CL_SUCCESS) {
        LOG(ERROR) << "Call clCreateCommandQueueWithProperties failed "
                    << "with code: " << error_code;
        return false;
    }
    else {
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
    error_code = clGetCommandQueueInfo(queue_, CL_QUEUE_PROPERTIES,
                                       sizeof(cl_command_queue_properties),
                                       (void*)queue_properties.data(), nullptr);
    if (error_code != CL_SUCCESS) {
        LOG(ERROR) << "Call clGetCommandQueueInfo failed with code: "
                   << error_code;
        return false;
    }

    Device* device = getSharedDevice();
    int opencl_version = device->getOpenCLVersion();
    if (opencl_version < 200) {
        if (queue_properties[0] == CL_QUEUE_PROFILING_ENABLE) {
            profiling_ = true;
        }
        else {
            profiling_ = false;
        }
    }
    else {
        if ((queue_properties[1] & CL_QUEUE_PROFILING_ENABLE) ||
            (queue_properties[3] & CL_QUEUE_PROFILING_ENABLE)) {
            profiling_ = true;
        }
        else {
            profiling_ = false;
        }
    }

    return true;
}

static FrameChain* shared_frame_chain;

void createSharedFrameChain(bool profiling) {
    static FrameChain frame_chain(profiling);

    shared_frame_chain = &frame_chain;
}

void createSharedFrameChain(const cl_command_queue& queue) {
    static FrameChain frame_chain(queue);

    shared_frame_chain = &frame_chain;
}

FrameChain* getSharedFrameChain() {
    return shared_frame_chain;
}

}}}
