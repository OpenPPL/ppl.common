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

#ifndef _ST_HPC_PPL_COMMON_OCL_KERNELBINARIESMANAGER_H_
#define _ST_HPC_PPL_COMMON_OCL_KERNELBINARIESMANAGER_H_

#include <string>
#include <mutex>
#include <map>
#include <utility>
#include <stddef.h>
#include <stdint.h>

#include "framechain.h"
#include "types_interface.h"

namespace ppl { namespace common { namespace ocl {

#define PROJECT_LENGTH 8
#define FUNCTION_LENGTH 16
#define KERNEL_LENGTH 32
#define BINARIES_FILE "kernel_binaries.db"

struct KernelBinaryInfo {
    uint32_t address_offset;
    uint32_t binaries_size;
};

struct Kernel2FunctionItem {
    char kernel_name[KERNEL_LENGTH];
    char project_name[PROJECT_LENGTH];
    char function_name[FUNCTION_LENGTH];
};

struct Function2BinariesItem {
    char project_name[PROJECT_LENGTH];
    char function_name[FUNCTION_LENGTH];
    uint32_t address_offset;
    uint32_t binaries_size;
};

class KernelBinariesManager {
public:
    KernelBinariesManager();
    ~KernelBinariesManager();

    bool isWorking() const {
        return is_working_;
    }
    void setStatus(bool is_working);
    bool prepareManager(BinariesManagerStatus status);
    bool buildFunctionFromSource(FrameChain* frame_chain);
    bool storeFunctionBinaries(FrameChain* frame_chain);
    bool storeMapstoFile();

    bool loadBinariesInfo();
    bool retrieveKernel(const std::string& project_name, const std::string& kernel_name, size_t* binaries_length,
                        unsigned char** binaries_data);
    void releaseResource();

private:
    FILE* fp_;
    uint32_t kernel_offset_;
    uint32_t kernel_count_;
    uint32_t function_offset_;
    uint32_t function_count_;
    bool is_working_;
    std::mutex locker_;
    // (kernel name, (project name, function name))
    std::map<std::string, std::pair<std::string, std::string>> kernel2function_;
    // (project name, (function name, kernel binaries info))
    std::map<std::string, std::map<std::string, KernelBinaryInfo>> function2binariesinfo_;
};

}}} // namespace ppl::common::ocl

#endif
