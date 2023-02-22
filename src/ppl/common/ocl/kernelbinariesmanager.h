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
#include <cstdio>
#include <map>

#include "framechain.h"

namespace ppl { namespace common { namespace ocl {

#define LENGTH 32
#define BINARIES_FILE "kernel_binaries.db"
#define PROCESS_A_FUNCTION(frame_chain, kernel_name)                           \
          processAFunction(frame_chain, kernel_name ## _string);

enum BinariesManagerStatus {
    BINARIES_COMPILE  = 0,
    BINARIES_RETRIEVE = 1,
};

struct KernelBinaryInfo {
  size_t address_offset;
	size_t size;
};

struct KernelBinaryItem {
  char project_name[LENGTH];
  char function_name[LENGTH];
  size_t address_offset;
	size_t size;
};

class KernelBinariesManager {
  public:
    KernelBinariesManager();
    ~KernelBinariesManager();

    bool prepareManager(BinariesManagerStatus status);
    bool buildFunctionFromSource(FrameChain* frame_chain,
                                 const char* source_str);
    bool storeFunctionBinaries(FrameChain* frame_chain);
    bool storeMaptoFile();

    bool loadMapfromFile();
    bool retrieveKernel(const std::string &project_name,
                        const std::string &kernel_name, size_t* binaries_length,
                        unsigned char* binaries_data);
    void destroyMap();

  private:
    std::mutex locker_;
    FILE* fp_;
    size_t binaries_offset_;
    size_t function_number_;
    std::map<std::string,
             std::map<std::string, KernelBinaryInfo>> function2kernelbinaries_;
};

bool initializeKernelBinariesManager(BinariesManagerStatus status);
bool processAFunction(FrameChain* frame_chain, const char* source_str);
bool detectKernelBinariesFile();
bool restoreKernelBianriesMap();
bool retrieveKernel(const std::string &kernel_name, size_t* binaries_length,
                    unsigned char* binaries_data);
bool shutDownKernelBinariesManager(BinariesManagerStatus status);

}}}

#endif
