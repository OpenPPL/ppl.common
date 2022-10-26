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
    // LOG(INFO) << "before free all kernel in kernel pool.";
    removeAllKernels();
    // LOG(INFO) << "after free all kernel in kernel pool.";
}

bool KernelPool::insertKernel(const cl_context &context,
                              const std::string &project_name,
                              const std::string &kernel_name,
                              const cl_kernel &kernel) {
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

    // LOG(INFO) << "In insertKernel, project name size: " << project_name.size();
    // LOG(INFO) << "In insertKernel, kernel name size: " << kernel_name.size();

    cl_int error_code;
    std::lock_guard<std::mutex> lock_guard(locker_);
    auto key = std::make_pair(context, project_name);
    auto iter0 = context2kernels_.find(key);
    if (iter0 == context2kernels_.end()) {
        error_code = clRetainContext(context);
        if (error_code != CL_SUCCESS) {
            LOG(ERROR) << "Call clRetainContext() failed with code: "
                       << error_code;
            error_code = clReleaseContext(context);
            if (error_code != CL_SUCCESS) {
                LOG(ERROR) << "Call clReleaseContext() failed with code: "
                           << error_code;
            }
            return false;
        }
    }

    auto &name2kernel = context2kernels_[key];
    auto iter1 = name2kernel.find(kernel_name);
    if (iter1 != name2kernel.end()) {
        // LOG(INFO) << "Kernel " << (void*)context << ", " << project_name << "::" << kernel_name
        //           << " has already existed in kernel pool.";
        return true;
    }

    name2kernel[kernel_name] = kernel;
    error_code = clRetainKernel(kernel);
    if (error_code != CL_SUCCESS) {
        name2kernel.erase(kernel_name);
        if (context2kernels_[key].size() == 0) {
            context2kernels_.erase(key);
            error_code = clReleaseContext(context);
            if (error_code != CL_SUCCESS) {
                LOG(ERROR) << "Call clReleaseContext() failed with code: "
                           << error_code;
            }
        }
        LOG(ERROR) << "Call clRetainKernel() failed with code: "
                   << error_code;
        return false;
    }
    // LOG(INFO) << "Insert kernel " << (void*)context << ", " << project_name << "::" << kernel_name
    //           << " into the kernel pool.";

    return true;
}

cl_kernel KernelPool::getKernel(const cl_context &context,
                                const std::string &project_name,
                                const std::string &kernel_name) {
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
    // LOG(INFO) << "context2kernels_ size: " << context2kernels_.size();
    if (iter0 == context2kernels_.end()) {
        // LOG(INFO) << "Kernel " << (void*)context << ", " << project_name << "::" << kernel_name
        //           << " does not exist in kernel pool.";
        return nullptr;
    }
    // LOG(INFO) << "context2kernels_ 0 key: " << (void*)(context2kernels_.begin()->first.first)
    //           << ", " << context2kernels_.begin()->first.second;

    auto &name2kernel = context2kernels_[key];
    auto iter1 = name2kernel.find(kernel_name);
    // auto number = name2kernel.count(kernel_name);
    // LOG(INFO) << "number of kernel name: " << number;
    // LOG(INFO) << "kernel_name: " << kernel_name;
    // LOG(INFO) << "name2kernel size: " << name2kernel.size();
    // LOG(INFO) << "name2kernel name: " << name2kernel.begin()->first;
    // LOG(INFO) << "name2kernel kernel: " << (void*)(name2kernel.begin()->second);
    // auto key_iter = name2kernel.begin();
    // while (key_iter != name2kernel.end()) {
    //     LOG(INFO) << "In iteration, kernel size: " << (key_iter->first).size();
    //     LOG(INFO) << "In iteration, kernel size: " << kernel_name.size();
    //     if (key_iter->first == kernel_name) {
    //         LOG(INFO) << "iteration found the kernel.";
    //         break;
    //     }
    //     key_iter++;
    // }
    // LOG(INFO) << "name2kernel name: " << iter1->first;
    if (iter1 == name2kernel.end()) {
        // LOG(INFO) << "Kernel " << (void*)context << ", " << project_name << "::" << kernel_name
        //           << " does not exist in kernel pool.";
        return nullptr;
    }

    return name2kernel[kernel_name];
}

bool KernelPool::removeKernel(const cl_context &context,
                              const std::string &project_name,
                              const std::string &kernel_name) {
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
        // LOG(INFO) << "Kernel " << project_name << "::" << kernel_name
        //           << " does not exist in kernel pool.";
        return false;
    }

    auto &name2kernel = context2kernels_[key];
    auto iter1 = name2kernel.find(kernel_name);
    if (iter1 == name2kernel.end()) {
        // LOG(INFO) << "Kernel " << project_name << "::" << kernel_name
        //           << " does not exist in kernel pool.";
        return false;
    }

    cl_int error_code;
    cl_kernel kernel = name2kernel[kernel_name];
    name2kernel.erase(kernel_name);
    error_code = clReleaseKernel(kernel);
    if (error_code != CL_SUCCESS) {
        LOG(ERROR) << "Call clReleaseKernel() failed with code: "
                   << error_code;
        return false;
    }

    if (name2kernel.size() == 0) {
        context2kernels_.erase(key);
    }
    error_code = clReleaseContext(context);
    if (error_code != CL_SUCCESS) {
        LOG(ERROR) << "Call clReleaseContext() failed with code: "
                    << error_code;
        return false;
    }

    return true;
}

bool KernelPool::removeAllKernels() {
    cl_int error_code;
    bool succeeded = true;

    // LOG(INFO) << "context2kernels_ size: " << context2kernels_.size();
    std::lock_guard<std::mutex> lock_guard(locker_);
    auto iter0 = context2kernels_.begin();
    while (iter0 != context2kernels_.end()) {
        auto &name2kernel = iter0->second;
        // LOG(INFO) << "context2kernels_ key: " << (void*)(iter0->first.first)
        //     << ", " << iter0->first.second;
        // LOG(INFO) << "name2kernel size: " << name2kernel.size();
        auto iter1 = name2kernel.begin();
        while (iter1 != name2kernel.end()) {
            // auto &kernel_name = iter1->first;
            cl_kernel kernel = iter1->second;
            // LOG(INFO) << "name2kernel: " << iter1->first
            //     << ", " << (void*)(iter1->second);

/*             size_t returned_size;
            error_code = clGetKernelInfo(kernel, CL_KERNEL_FUNCTION_NAME, 0, nullptr,
                                        &returned_size);
            if (error_code != CL_SUCCESS) {
                LOG(ERROR) << "Call clGetKernelInfo() failed with code: "
                        << error_code;
                return false;
            }

            if (returned_size <= 0) {
                LOG(ERROR) << "No kernel is detected by clGetKernelInfo().";
                return false;
            }

            std::string param_value;
            param_value.resize(returned_size);
            error_code = clGetKernelInfo(kernel, CL_KERNEL_FUNCTION_NAME, returned_size,
                                        (void*)param_value.data(), nullptr);
            if (error_code != CL_SUCCESS) {
                LOG(ERROR) << "Call clGetKernelInfo() failed with code: "
                        << error_code;
                return false;
            }
            LOG(INFO) << "  kernel function name: " << param_value; */


            // name2kernel.erase(kernel_name);
            // LOG(INFO) << "  before clReleaseKernel() ";
            error_code = clReleaseKernel(kernel);
            // LOG(INFO) << "  after clReleaseKernel() ";
            if (error_code != CL_SUCCESS) {
                LOG(ERROR) << "Call clReleaseKernel() failed with code: "
                           << error_code;
                succeeded = false;
            }
            // LOG(INFO) << "  before name2kernel.erase() ";
            iter1 = name2kernel.erase(iter1);
            // LOG(INFO) << "  after name2kernel.erase() ";
        }

        auto &key = iter0->first;
        auto context = key.first;
        // LOG(INFO) << "  before clReleaseContext() ";
        error_code = clReleaseContext(context);
        // LOG(INFO) << "  after clReleaseContext() ";
        if (error_code != CL_SUCCESS) {
            LOG(ERROR) << "Call clReleaseContext() failed with code: "
                       << error_code;
            return false;
        }
        iter0 = context2kernels_.erase(iter0);
    }

    return succeeded;
}

static KernelPool kernel_pool;

// KernelPool* getKernelPool() {
//     static KernelPool instance();

//     return &instance;
// }

bool insertKernelToPool(const cl_context &context,
                        const std::string &project_name,
                        const std::string &kernel_name,
                        const cl_kernel &kernel) {
    return kernel_pool.insertKernel(context, project_name, kernel_name, kernel);
}

cl_kernel getKernelFromPool(const cl_context &context,
                            const std::string &project_name,
                            const std::string &kernel_name) {
    return kernel_pool.getKernel(context, project_name, kernel_name);
}

bool removeKernelFromPool(const cl_context &context,
                          const std::string &project_name,
                          const std::string &kernel_name) {
    return kernel_pool.removeKernel(context, project_name, kernel_name);
}

}}}
