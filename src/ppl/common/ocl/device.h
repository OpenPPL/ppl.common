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

#ifndef _ST_HPC_PPL_COMMON_OCL_DEVICE_H_
#define _ST_HPC_PPL_COMMON_OCL_DEVICE_H_

#include <vector>
#include <string>

#include "openclruntime.h"

namespace ppl { namespace common { namespace ocl {

#define PARAM_SIZE 64

enum GpuTypes {
    ADRENO_GPU,
    MALI_GPU,
    INTEL_GPU,
    NVIDIA_GPU,
    AMD_GPU,
    OTHER_GPU,
    INVALID_GPU,
};

class Device {
public:
    Device();
    Device(int platform_index, int device_index);
    ~Device();

    bool detectPlatforms();
    int getPlatformNum() const;
    void setPlatformIndex(int index);
    bool detectDevices();
    int getDeviceNum() const;
    void setDeviceIndex(int index);
    bool detectAValidPlatformDevice();

    cl_platform_id getPlatformId();
    cl_platform_id getPlatformId(int index);
    cl_device_id getDeviceId();
    cl_device_id getDeviceId(int index);

    bool getPlatformInfos();
    bool getDeviceBasicInfos(const cl_device_id& device_id);
    bool getDeviceBasicInfos();
    bool getDeviceThoroughInfos(const cl_device_id& device_id);
    bool getDeviceThoroughInfos();

    GpuTypes checkGpuType(const cl_device_id& device_id);
    int checkOpenCLVersion(const cl_device_id& device_id);

    GpuTypes getGpuType() const {
        return gpu_type_;
    }
    int getOpenCLVersion() const {
        return opencl_version_;
    }
    cl_uint getCUNum() const {
        return compute_units_;
    }
    cl_uint getMaxWorkDims() const {
        return max_work_dim_;
    }
    size_t getMaxWorkItemsInGroup() const {
        return max_items_in_group_;
    }
    std::vector<size_t> getMaxItemsPerGroupDim() const {
        return max_group_items_per_dim_;
    }
    cl_ulong getGlobalMemSize() const {
        return global_mem_size_;
    }
    size_t getMaxImageWidth() {
        return this->image3d_max_width_;
    }
    size_t getMaxImageHeight() {
        return this->image3d_max_height_;
    }
    size_t getMaxImageDepth() {
        return this->image3d_max_depth_;
    }
    size_t getMaxMemAllocSize() {
        return this->max_mem_alloc_size_;
    }
    size_t getLocalMemSize() {
        return this->local_mem_size_;
    }
    std::string getDeviceName() {
        return this->device_name_;
    }

private:
    bool queryPlatformInfo(const cl_platform_id& platform_id, cl_platform_info param_name, std::string& param_value);
    bool queryDeviceInfo(const cl_device_id& device_id, cl_device_info param_name, std::string& param_value,
                         bool scaling);

protected:
    bool platform_detected_;
    bool device_detected_;
    int platform_index_;
    int device_index_;

    GpuTypes gpu_type_;
    int opencl_version_;

    std::vector<cl_platform_id> platform_ids_;
    std::string platform_name_;
    std::string platform_vendor_;
    std::string platform_profile_;
    std::string platform_version_;
    std::string platform_extensions_;

    std::vector<cl_device_id> device_ids_;
    std::string device_name_;
    std::string device_vendor_;
    std::string device_profile_;
    std::string device_version_;
    std::string driver_version_;
    std::string device_opencl_c_version_;
    std::string device_extensions_;

    std::string device_il_version_;
    std::string built_in_kernels_;

    cl_uint compute_units_;
    cl_uint max_work_dim_;
    size_t max_items_in_group_;
    std::vector<size_t> max_group_items_per_dim_;

    cl_int native_vector_width_char_;
    cl_int native_vector_width_short_;
    cl_int native_vector_width_int_;
    cl_int native_vector_width_long_;
    cl_int native_vector_width_half_;
    cl_int native_vector_width_float_;
    cl_int native_vector_width_double_;

    cl_int preferred_vector_width_char_;
    cl_int preferred_vector_width_short_;
    cl_int preferred_vector_width_int_;
    cl_int preferred_vector_width_long_;
    cl_int preferred_vector_width_half_;
    cl_int preferred_vector_width_float_;
    cl_int preferred_vector_width_double_;

    cl_device_fp_config single_fp_config_;
    cl_device_fp_config double_fp_config_;

    cl_ulong max_mem_alloc_size_;
    cl_ulong global_mem_size_;
    cl_uint global_mem_cacheline_size_;
    cl_ulong global_mem_cache_size_;
    cl_device_mem_cache_type global_mem_cache_type_;

    bool image_supported_;
    cl_uint max_read_image_args_;
    cl_uint max_write_image_args_;
    cl_uint max_read_write_image_args_;
    size_t image2d_max_width_;
    size_t image2d_max_height_;
    size_t image3d_max_width_;
    size_t image3d_max_height_;
    size_t image3d_max_depth_;
    cl_uint max_samplers_;

    cl_ulong constant_buffer_size_;
    cl_uint max_constant_args_;
    size_t max_global_variable_size_;
    cl_device_local_mem_type local_mem_type_;
    cl_ulong local_mem_size_;

    cl_uint max_clock_frequency_;
    cl_uint address_bits_;
    size_t max_parameter_size_;

    cl_bool endian_little_;
    cl_bool device_available_;
    cl_bool compiler_abailable_;
    cl_bool linker_abailable_;

    cl_command_queue_properties host_queue_properties_;
    cl_command_queue_properties device_queue_properties_;
#if CL_TARGET_OPENCL_VERSION >= 200
    cl_device_svm_capabilities svm_capabilities_;
#endif
};

void createSharedDevice();
void createSharedDevice(int platform_index, int device_index);
Device* getSharedDevice();

size_t GetMaxImageWidth();
size_t GetMaxImageHeight();
size_t GetMaxImageDepth();

}}} // namespace ppl::common::ocl

#endif
