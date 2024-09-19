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
#include <vector>

#include "openclruntime.h"

namespace ppl { namespace common { namespace ocl {

#define SET_PROGRAM_SOURCE(frame_chain, source)  \
    {                                            \
        frame_chain->setSourceFileName(#source); \
        frame_chain->setSource(source##_string); \
    }

#define SET_PROGRAM_SOURCE_FILE(frame_chain, source) \
    { frame_chain->setSourceFileName(#source); }

enum CreatingProgramTypes {
    WITH_SOURCE = 0,
    WITH_IL = 1,
    WITH_BINARIES = 2,
    WITH_BUILT_IN_KERNELS = 3,
};

enum PlatformType0 {
    PlatformType0_QCOM = 0,
    PlatformType0_ARM = 1,
    PlatformType0_INTEL = 2,
    PlatformType0_invalid = 0xffffffff,
};

struct QCOM_ext {
    bool is_support_reqd_sub_group_size = false;
    bool is_support_subgroup_shuffle = false;
};

struct INTEL_ext {
    //shuffle vectors and rotate2
    bool is_support_intel_enhanced_shuffle = false;
};

union PlatformOnly_ext {
    struct QCOM_ext;
    struct ARM_ext;
    struct INTEL_ext;
};

typedef struct eventNode {
    cl_event event;
    std::string kernel_name;

    eventNode(cl_event evt, std::string name) : event(evt), kernel_name(name) {}
} eventNode;

class FrameChain {
public:
    FrameChain(bool profiling, int perf_hint, int priority_hint);
    FrameChain(const cl_command_queue& queue);
    ~FrameChain();

    std::vector<eventNode> event_list;
    void printEventList();
    void releaseEventList();
    void setProgram(const cl_program program);
    void setCreatingProgramType(const CreatingProgramTypes creating_program_type);
    void setSaveProgramBinaryFlag(bool save_program_binary);
    void setOptLevel(uint32_t opt_level);
    void setSourceFileName(const char* source_file_name);
    void setSource(const char* source_string);
    void setSpir(const void* spir_string, size_t spir_size);
    void setProjectName(const char* project_name);
    void setFunctionName(const char* function_string);
    void setCompileOptions(const char* options);
    void setKernelTime(uint64_t kernel_time) {
        kernel_time_ = kernel_time;
    }
    void setTuningQueueStatus(bool on) {
        tuning_queue_on_ = on;
    }

    cl_platform_id getPlatformId() const {
        return platform_id_;
    }
    cl_device_id getDeviceId() const {
        return device_id_;
    }
    cl_context getContext() const {
        return context_;
    }
    cl_command_queue getQueue() const {
        return queue_;
    }
    cl_program getProgram() const {
        return program_;
    }
    CreatingProgramTypes getCreatingProgramType() const {
        return creating_program_type_;
    }
    char* getSourceFileName() const {
        return source_file_name_;
    }
    char* getCodeString() const {
        return source_string_;
    }
    void* getSpirString() const {
        return spir_string_;
    }
    size_t getSpirSize() const {
        return spir_size_;
    }
    std::string getProjectName() const {
        return project_name_;
    }
    std::string getFunctionName() const {
        return function_name_;
    }
    std::string getCompileOptions() const {
        return compile_options_;
    }
    std::string getDeviceDesc() const {
        return device_desc_;
    }
    std::string getVendorDesc() const {
        return vendor_desc_;
    }
    bool isProfiling() const {
        return profiling_;
    }
    bool getSaveProgramBinaryFlag() const {
        return save_program_binary_;
    }
    uint32_t getOptLevel() const {
        return opt_level_;
    }
    uint64_t getKernelTime() const {
        return kernel_time_;
    }
    bool getTuningQueueStatus() const {
        return tuning_queue_on_;
    }
    cl_command_queue getTuningQueue();

    bool ifSupportQcomHints();

    // extention related interfaces
    void get_extention_info();

    size_t getMaxSubGroupSize() {
        return max_subgroup_size_;
    }
    bool isSupportFp16() {
        return is_support_fp16;
    };
    bool isSupportSubgroup() {
        return is_support_subgroup;
    }
    bool isSupport3DImageWrite() {
        return is_support_3d_image_write;
    }
    bool isSupportInt8Product() {
        return is_support_int8_product;
    }

    bool isSupportSubgroupShuffle() {
        return is_support_subgroup_shuffle;
    }
    bool isSupportSubgroupRotate() {
        return is_support_subgroup_rotate;
    }


    PlatformType0 getPlatformType() {
        return platform_type0;
    }

    QCOM_ext* getQcomExtInfo() {
        return (QCOM_ext*)&PlatformOnly_ext_info;
    }
    INTEL_ext* getIntelExtInfo() {
        return (INTEL_ext*)&PlatformOnly_ext_info;
    }

    // todo, other platforms

protected:
    bool createDefaultOclFrame(bool profiling, int perf_hint, int priority_hint);
    bool queryProfiling();

private:
    // shared by all functions/program.
    cl_platform_id platform_id_;
    cl_device_id device_id_;
    cl_context context_;
    cl_command_queue queue_;
    cl_command_queue tuning_queue_;
    std::string device_desc_;
    std::string vendor_desc_;

    // unique to each function/program.
    cl_program program_;
    CreatingProgramTypes creating_program_type_;
    char* source_file_name_;
    char* source_string_;
    void* spir_string_;
    size_t spir_size_;
    std::string project_name_;
    std::string function_name_;
    std::string compile_options_;
    std::string compile_options_ext_defaults_;
    bool profiling_;
    bool save_program_binary_;
    uint32_t opt_level_;
    uint64_t kernel_time_;
    bool tuning_queue_on_;

    //// platform extentions
    size_t max_subgroup_size_ = 0;

    bool is_support_fp16 = false;
    bool is_support_subgroup = false;
    bool is_support_3d_image_write = false;
    bool is_support_int8_product = false;

    bool is_support_subgroup_shuffle = false;
    bool is_support_subgroup_rotate = false;

    PlatformType0 platform_type0 = PlatformType0_invalid; // 0 qcom, 1 arm ...
    PlatformOnly_ext PlatformOnly_ext_info;
};

void createSharedFrameChain(bool profiling, int perf_hint, int priority_hint);
void createSharedFrameChain(const cl_command_queue& queue);
FrameChain* getSharedFrameChain();

}}} // namespace ppl::common::ocl

#endif
