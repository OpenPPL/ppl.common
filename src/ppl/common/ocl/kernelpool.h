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

#ifndef _ST_HPC_PPL_COMMON_OCL_KERNELPOOL_H_
#define _ST_HPC_PPL_COMMON_OCL_KERNELPOOL_H_

#include <mutex>
#include <map>
#include <utility>
#include <string>

#include "CL/cl.h"

namespace ppl { namespace common { namespace ocl {

class KernelPool {
  public:
    KernelPool();
    ~KernelPool();

    bool insertKernel(const cl_context &context,
                      const std::string &project_name,
                      const std::string &kernel_name,
                      const cl_kernel &kernel);
    cl_kernel getKernel(const cl_context &context,
                        const std::string &project_name,
                        const std::string &kernel_name);
    bool removeKernel(const cl_context &context,
                      const std::string &project_name,
                      const std::string &kernel_name);
    bool removeAllKernels();

  private:
    std::mutex locker_;
    std::map<std::pair<cl_context, std::string>,
             std::map<std::string, cl_kernel>> context2kernels_;
};

bool insertKernelToPool(const cl_context &context,
                        const std::string &project_name,
                        const std::string &kernel_name,
                        const cl_kernel &kernel);
cl_kernel getKernelFromPool(const cl_context &context,
                            const std::string &project_name,
                            const std::string &kernel_name);
bool removeKernelFromPool(const cl_context &context,
                          const std::string &project_name,
                          const std::string &kernel_name);

}}}

#endif
