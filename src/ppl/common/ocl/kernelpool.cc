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

#include "kernelpool.h"

#include "ppl/common/log.h"

namespace ppl { namespace common { namespace ocl {

KernelPool::KernelPool() {}

KernelPool::~KernelPool() {
    // This function should be used before deleting cl_context and command queue.
    if (!context2kernels_.empty()) {
        removeAllKernels();
    }
}

bool KernelPool::insertKernel(const cl_context& context, const std::string& project_name,
                              const std::string& kernel_name, const cl_kernel& kernel) {
    if (context == nullptr) {
        LOG(ERROR) << "The context is invalid.";
        return false;
    }

    if (project_name.size() == 0) {
        LOG(ERROR) << "The project name is invalid.";
        return false;
    }

    if (kernel_name.size() == 0) {
        LOG(ERROR) << "The kernel name is invalid.";
        return false;
    }

    if (kernel == nullptr) {
        LOG(ERROR) << "The kernel is invalid.";
        return false;
    }

    std::lock_guard<std::mutex> lock_guard(locker_);
    auto key = std::make_pair(context, project_name);
    auto& name2kernel = context2kernels_[key];
    auto iter1 = name2kernel.find(kernel_name);
    if (iter1 != name2kernel.end()) {
        return true;
    }
    name2kernel[kernel_name] = kernel;

    return true;
}

cl_kernel KernelPool::getKernel(const cl_context& context, const std::string& project_name,
                                const std::string& kernel_name) {
    if (context == nullptr) {
        LOG(ERROR) << "The context is invalid.";
        return nullptr;
    }

    if (project_name.size() == 0) {
        LOG(ERROR) << "The project name is invalid.";
        return nullptr;
    }

    if (kernel_name.size() == 0) {
        LOG(ERROR) << "The kernel name is invalid.";
        return nullptr;
    }

    std::lock_guard<std::mutex> lock_guard(locker_);
    auto key = std::make_pair(context, project_name);
    auto iter0 = context2kernels_.find(key);
    if (iter0 == context2kernels_.end()) {
        return nullptr;
    }

    auto& name2kernel = context2kernels_[key];
    auto iter1 = name2kernel.find(kernel_name);
    if (iter1 == name2kernel.end()) {
        return nullptr;
    }

    return name2kernel[kernel_name];
}

bool KernelPool::removeKernel(const cl_context& context, const std::string& project_name,
                              const std::string& kernel_name) {
    if (context == nullptr) {
        LOG(ERROR) << "The context is invalid.";
        return false;
    }

    if (project_name.size() == 0) {
        LOG(ERROR) << "The project name is invalid.";
        return false;
    }

    if (kernel_name.size() == 0) {
        LOG(ERROR) << "The kernel name is invalid.";
        return false;
    }

    std::lock_guard<std::mutex> lock_guard(locker_);
    auto key = std::make_pair(context, project_name);
    auto iter0 = context2kernels_.find(key);
    if (iter0 == context2kernels_.end()) {
        return false;
    }

    auto& name2kernel = context2kernels_[key];
    auto iter1 = name2kernel.find(kernel_name);
    if (iter1 == name2kernel.end()) {
        return false;
    }

    cl_int error_code;
    cl_kernel kernel = name2kernel[kernel_name];
    name2kernel.erase(kernel_name);
    error_code = clReleaseKernel(kernel);
    if (error_code != CL_SUCCESS) {
        LOG(ERROR) << "Call clReleaseKernel() failed with code: " << error_code;
        return false;
    }

    if (name2kernel.size() == 0) {
        context2kernels_.erase(key);
    }

    return true;
}

bool KernelPool::removeAllKernels() {
    cl_int error_code;
    bool succeeded = true;

    std::lock_guard<std::mutex> lock_guard(locker_);
    auto iter0 = context2kernels_.begin();
    while (iter0 != context2kernels_.end()) {
        auto& name2kernel = iter0->second;
        auto iter1 = name2kernel.begin();
        while (iter1 != name2kernel.end()) {
            cl_kernel kernel = iter1->second;
            error_code = clReleaseKernel(kernel);
            if (error_code != CL_SUCCESS) {
                LOG(ERROR) << "Call clReleaseKernel() failed with code: " << error_code;
                succeeded = false;
            }
            iter1 = name2kernel.erase(iter1);
        }

        iter0 = context2kernels_.erase(iter0);
    }
    context2kernels_.clear();
    return succeeded;
}

static KernelPool kernel_pool;

bool insertKernelToPool(const cl_context& context, const std::string& project_name, const std::string& kernel_name,
                        const cl_kernel& kernel) {
    return kernel_pool.insertKernel(context, project_name, kernel_name, kernel);
}

cl_kernel getKernelFromPool(const cl_context& context, const std::string& project_name,
                            const std::string& kernel_name) {
    return kernel_pool.getKernel(context, project_name, kernel_name);
}

bool removeKernelFromPool(const cl_context& context, const std::string& project_name, const std::string& kernel_name) {
    return kernel_pool.removeKernel(context, project_name, kernel_name);
}

bool removeAllKernelsFromPool() {
    return kernel_pool.removeAllKernels();
}

}}} // namespace ppl::common::ocl
