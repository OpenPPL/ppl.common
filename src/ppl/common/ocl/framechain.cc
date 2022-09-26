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

FrameChain::FrameChain() : platform_id_(nullptr), 
        device_id_(nullptr), context_(nullptr), queue_(nullptr), 
        program_(nullptr), profiling_(false) {
}

FrameChain::FrameChain(const cl_command_queue& queue) : platform_id_(nullptr), 
        device_id_(nullptr), context_(nullptr), program_(nullptr), 
        profiling_(false) {
    if (queue == nullptr) {
        LOG(ERROR) << "Invalid command queue.";
    }

    queue_ = queue;
}

FrameChain::~FrameChain() {
}

void FrameChain::setSource(char* source_string) {
    if (source_string == nullptr) {
        LOG(ERROR) << "Invalid address of the source string.";
    }

    source_string_ = source_string;
}

void FrameChain::setFunctionName(const char* function_name) {
    if (function_name == nullptr) {
        LOG(ERROR) << "Invalid address of the function name.";
    }

    strcpy(function_name_, function_name);
}

void FrameChain::setProgram(const cl_program& program) {
    if (program == nullptr) {
        LOG(ERROR) << "Invalid OpenCL program.";
    }

    program_ = program;
}

bool FrameChain::createDefaultOclFrame(bool profiling) {
    cl_int error_code;
    bool succeeded = false;
    Device device();
    device.detectPlatforms();

    for (int index = 0; index < device.getPlatformNum(); index++) {
        device.setPlatformIndex(index);
        device.detectDevices();
        if (device.getDeviceNum() == 0) {
            continue;
        }
        cl_device_id device_id = device.getDeviceId(0);
        
        std::vector<cl_context_properties> context_properties;
        context_properties.resize(3);
        context_properties[0] = CL_CONTEXT_PLATFORM;
        context_properties[1] = device.getPlatformId(index);
        context_properties[2] = 0;
        context_ = clCreateContext(context_properties.data(), 1, &device_id,  
                                   nullptr, nullptr, &error_code);
        if (error_code != CL_SUCCESS) {
          LOG(ERROR) << "Call clCreateContext failed with code: " << error_code;
          return false;            
        }

        succeeded = true;
        platform_id_ = device.getPlatformId(index);
        device_id_ = device_id;
        break;
    }

    if (succeeded == false) {
        LOG(ERROR) << "No valid opencl platform and device is detected.";
        return false;
    }

    profiling_ = profiling;
    int opencl_version = device.checkOpenCLVersion(device_id_);
    if (opencl_version < 200) {
        cl_command_queue_properties queue_properties;
        if (profiling) {
            queue_properties = CL_QUEUE_PROFILING_ENABLE;
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
    }
    else {
        std::vector<cl_queue_properties> queue_properties;
        queue_properties.resize(3);
        queue_properties[0] = CL_QUEUE_PROPERTIES;
        if (profiling) {
            queue_properties[1] = CL_QUEUE_PROFILING_ENABLE;
        }
        queue_properties[2] = 0;
        queue_ = clCreateCommandQueueWithProperties(context_, device_id_, 
                     &queue_properties, &error_code);
        if (error_code != CL_SUCCESS) {
            LOG(ERROR) << "Call clCreateCommandQueueWithProperties failed " 
                       << "with code: " << error_code;
            return false;            
        }
        else {
            return true;
        }
    }
}

bool FrameChain::queryDevice() {
    if (queue_ == nullptr) {
        LOG(ERROR) << "The command queue is uninitialized.";

        return false;        
    }

    if (device_id_ != nullptr) {
        return true;
    }

    cl_int error_code;
    error_code = clGetCommandQueueInfo(queue_, CL_QUEUE_DEVICE, 
                                       sizeof(cl_device_id), &device_id_, 
                                       nullptr);
    if (error_code != CL_SUCCESS) {
        LOG(ERROR) << "Call clGetCommandQueueInfo failed with code: " 
                   << error_code;
        return false;
    }

    return true;
}

bool FrameChain::queryContext() {
    if (queue_ == nullptr) {
        LOG(ERROR) << "The command queue is uninitialized.";

        return false;        
    }

    if (context_ != nullptr) {
        return true;
    }
    
    cl_int error_code;
    error_code = clGetCommandQueueInfo(queue_, CL_QUEUE_CONTEXT, 
                                       sizeof(cl_context), &context_, 
                                       nullptr);
    if (error_code != CL_SUCCESS) {
        LOG(ERROR) << "Call clGetCommandQueueInfo failed with code: " 
                   << error_code;
        return false;
    }

    return true;
}

bool FrameChain::queryProfiling() {
    if (queue_ == nullptr) {
        LOG(ERROR) << "The command queue is uninitialized.";

        return false;        
    }

    if (device_id_ == nullptr) {
        bool succeeded = queryDevice();
        if (!succeeded) {
            LOG(ERROR) << "Failed to query the profiling status of the " 
                       << "command queue.";
            return false;
        }
    }

    cl_int error_code;
    std::vector<cl_command_queue_properties> queue_properties(5);
    error_code = clGetCommandQueueInfo(queue_, CL_QUEUE_PROPERTIES, 
                                       sizeof(cl_command_queue_properties),  
                                       &queue_properties, nullptr);
    if (error_code != CL_SUCCESS) {
        LOG(ERROR) << "Call clGetCommandQueueInfo failed with code: " 
                << error_code;
        return false;
    }

    Device device();
    int opencl_version = device.checkOpenCLVersion(device_id_);
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

}}}    
