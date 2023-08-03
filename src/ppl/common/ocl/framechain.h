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

#ifndef _ST_HPC_PPL_COMMON_OCL_FRAMECHAIN_H_
#define _ST_HPC_PPL_COMMON_OCL_FRAMECHAIN_H_

#include <string>

#include "openclruntime.h"

namespace ppl { namespace common { namespace ocl {

#define SET_PROGRAM_SOURCE(frame_chain, source)                                \
        frame_chain->setSource(source ## _string);

enum CreatingProgramTypes {
    WITH_SOURCE = 0,
    WITH_IL = 1,
    WITH_BINARIES = 2,
    WITH_BUILT_IN_KERNELS = 3,
};

class FrameChain {
  public:
    FrameChain(bool profiling);
    FrameChain(const cl_command_queue& queue);
    ~FrameChain();

    void setProgram(const cl_program program);
    void setCreatingProgramType(const CreatingProgramTypes
                                creating_program_type);
    void setSource(const char* source_string);
    void setSpir(const void* spir_string, size_t spir_size);
    void setProjectName(const char* project_name);
    void setFunctionName(const char* function_string);
    void setCompileOptions(const char* options);

    cl_platform_id getPlatformId() const { return platform_id_; }
    cl_device_id getDeviceId() const { return device_id_; }
    cl_context getContext() const { return context_; }
    cl_command_queue getQueue() const { return queue_; }
    cl_program getProgram() const { return program_; }
    CreatingProgramTypes getCreatingProgramType() const {
      return creating_program_type_;
    }
    char* getCodeString() const { return source_string_; }
    void* getSpirString() const { return spir_string_; }
    size_t getSpirSize() const { return spir_size_; }
    std::string getProjectName() const { return project_name_; }
    std::string getFunctionName() const { return function_name_; }
    std::string getCompileOptions() const { return compile_options_; }
    bool isProfiling() const { return profiling_; }

  protected:
    bool createDefaultOclFrame(bool profiling);
    bool queryProfiling();

  private:
    // shared by all functions/program.
    cl_platform_id platform_id_;
    cl_device_id device_id_;
    cl_context context_;
    cl_command_queue queue_;

    // unique to each function/program.
    cl_program program_;
    CreatingProgramTypes creating_program_type_;
    char* source_string_;
    void* spir_string_;
    size_t spir_size_;
    std::string project_name_;
    std::string function_name_;
    std::string compile_options_;
    bool profiling_;
};

void createSharedFrameChain(bool profiling);
void createSharedFrameChain(const cl_command_queue& queue);
FrameChain* getSharedFrameChain();

}}}

#endif
