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

#include "device.h"

#include <string.h>
#include <algorithm>

#include "ppl/common/log.h"

namespace ppl { namespace common { namespace ocl {

bool Device::detectPlatforms() {
    cl_int error_code;
    cl_uint num_platforms;
    error_code = clGetPlatformIDs(MAX_ENTRIES, platform_ids_.data(), 
                                  &num_platforms);
    if (error_code != CL_SUCCESS) {
        LOG(ERROR) << "Call clGetPlatformIDs() failed with code: " 
                   << error_code;
        return false;
    }

    if (num_platforms > MAX_ENTRIES) {
        platform_ids_.resize(num_platforms);
        error_code = clGetPlatformIDs(num_platforms, platform_ids_.data(), 
                                      nullptr);
        if (error_code != CL_SUCCESS) {
            LOG(ERROR) << "Call clGetPlatformIDs() failed with code: " 
                       << error_code;
            return false;
        }        
    }

    if (num_platforms < MAX_ENTRIES) {
        platform_ids_.resize(num_platforms);
    }

    platform_detected_ = true;

    return true;
}

int Device::getPlatformNum() const {
    if (platform_detected_) {
        return platform_ids_.size();
    }
    else {
        return 0;
    }
}

void Device::setPlatformIndex(int index) {
    platform_id_ = index;
}

int Device::detectDevices() {
    if (device_ids_.size() != 0) {
        device_ids_.clear();
        device_ids_.resize(MAX_ENTRIES);
    }

    cl_int error_code;
    cl_uint num_devices;
    error_code = clGetDeviceIDs(platform_ids_[platform_id_], CL_DEVICE_TYPE_GPU,  
                                MAX_ENTRIES, device_ids_.data(), &num_devices);
    if (error_code != CL_SUCCESS) {
        LOG(ERROR) << "Call clGetDeviceIDs() failed with code: " << error_code;
        
        return false;
    }

    if (num_devices > MAX_ENTRIES) {
        device_ids_.resize(num_devices);
        error_code = clGetDeviceIDs(platform_ids_[platform_id_], 
                                    CL_DEVICE_TYPE_GPU, MAX_ENTRIES, 
                                    device_ids_.data(), nullptr);
        if (error_code != CL_SUCCESS) {
            LOG(ERROR) << "Call clGetDeviceIDs() failed with code: " 
                       << error_code;
            return false;
        }        
    }

    if (num_devices < MAX_ENTRIES) {
        device_ids_.resize(num_devices);
    }

    device_detected_ = true;

    return true;
}

int Device::getDeviceNum() const {
    if (device_detected_) {
        return device_ids_.size();
    }
    else {
        return 0;
    }
}

void Device::setDeviceIndex(int index) {
    device_id_ = index;
}

cl_platform_id Device::getPlatformId(int index) {
    return platform_ids_[index];
}

cl_device_id Device::getDeviceId(int index) {
    return device_ids_[index];
}

bool Device::queryPlatformInfo(const cl_platform_id& platform_id, 
                               cl_platform_info param_name, size_t value_size, 
                               std::string& param_value) {
    cl_int error_code;
    size_t returned_size;
    error_code = clGetPlatformInfo(platform_id, param_name, value_size,
                                   param_value.data(), &returned_size);
    if (error_code != CL_SUCCESS) {
        LOG(ERROR) << "Call clGetPlatformInfo() failed with code: " 
                   << error_code;
        
        return error_code;
    }

    if (returned_size > value_size) {  // must be ≥ size of return type specified in the Platform Queries table?
        param_value.resize(returned_size);
        error_code = clGetPlatformInfo(platform_id, param_name, returned_size,
                                       param_value.data(), nullptr);
        if (error_code != CL_SUCCESS) {
            LOG(ERROR) << "Call clGetPlatformInfo() failed with code: " 
                       << error_code;
            
            return error_code;
        }        
    }

    if (returned_size < value_size) {
        param_value.resize(returned_size);
    }

    return CL_SUCCESS;
}

bool Device::getPlatformInfos() {
    if (!platform_detected_) {
        LOG(ERROR) << "platforms has not been detected."; 

        return false;
    }

    bool succeeded;
    succeeded = queryPlatformInfo(platform_ids_[platform_id_], CL_PLATFORM_NAME, 
                                  PARAM_SIZE0, platform_name_.data());
    if (!succeeded) {
        return false;
    }    

    succeeded = queryPlatformInfo(platform_ids_[platform_id_],  
                                  CL_PLATFORM_VENDOR, PARAM_SIZE0, 
                                  platform_vendor_.data());
    if (!succeeded) {
        return false;
    }

/*     std::string vendor(platform_vendor_);
    std::transform(vendor.begin(), vendor.end(), vendor.begin(), ::tolower);
    if (vendor.find("qualcomm") != string::npos) {
        gpu_type_ = ADRENO_GPU;
    } 
    else if (vendor.find("arm") != string::npos) { // "mali" ?
        gpu_type_ = MALI_GPU;
    } 
    else {
       gpu_type_ = OTHER_GPU; 
    } */

    succeeded = queryPlatformInfo(platform_ids_[platform_id_], 
                                  CL_PLATFORM_PROFILE, PARAM_SIZE0, 
                                  platform_profile_.data());
    if (!succeeded) {
        return false;
    }

    succeeded = queryPlatformInfo(platform_ids_[platform_id_], 
                                  CL_PLATFORM_VERSION, PARAM_SIZE0, 
                                  platform_extensions_.data());
    if (!succeeded) {
        return false;
    }      

    succeeded = queryPlatformInfo(platform_ids_[platform_id_],  
                                  CL_PLATFORM_EXTENSIONS, PARAM_SIZE1, 
                                  platform_version_.data());
    if (!succeeded) {
        return false;
    }      

    return true;
}

bool Device::queryDeviceInfo(const cl_device_id& device_id, 
                             cl_device_info param_name, size_t value_size,
                             std::string& param_value, bool scaling) {
    cl_int error_code;
    size_t returned_size;
    error_code = clGetDeviceInfo(device_id, param_name, value_size,
                                 param_value.data(), &returned_size);
    if (error_code != CL_SUCCESS) {
        LOG(ERROR) << "Call clGetDeviceInfo() failed with code: " 
                   << error_code;
        
        return false;
    }

    if (returned_size > value_size) { // must be ≥ size of return type specified in the Device Queries table?
        param_value.resize(returned_size);
        error_code = clGetDeviceInfo(device_id, param_name, returned_size,
                                     param_value.data(), nullptr);
        if (error_code != CL_SUCCESS) {
            LOG(ERROR) << "Call clGetDeviceInfo() failed with code: " 
                       << error_code;
            
            return false;
        }        
    }

    if (scaling && returned_size < value_size) {
        param_value.resize(returned_size);
    }

    return true;
}

#define QUERY_DEVICE_INFO(succeeded, device_id, param_name, param_size,        \
                          param_value, scaling)                                \
    succeeded = queryDeviceInfo(device_id, param_name, param_size, param_value,\
                                scaling);                                      \
    if (!succeeded) {                                                          \
        return false;                                                          \
    }

GpuTypes Device::checkOpenCLType(const cl_device_id& device_id) {
    if (device_id == nullptr) {
        LOG(ERROR) << " Invalid device id."; 

        return -1;
    }

    bool succeeded;
    QUERY_DEVICE_INFO(succeeded, device_id, CL_DEVICE_VENDOR, PARAM_SIZE0, 
                      device_vendor_, true);
    // std::transform(device_vendor_.begin(), device_vendor_.end(), device_vendor_.begin(), ::tolower);
    if (device_vendor_.find("QUALCOMM") != string::npos) {
        gpu_type_ = ADRENO_GPU;
    } 
    else if (device_vendor_.find("ARM") != string::npos) { // "mali" ?
        gpu_type_ = MALI_GPU;
    } 
    else {
       gpu_type_ = OTHER_GPU; 
    }

    return gpu_type_;
}

int Device::checkOpenCLVersion(const cl_device_id& device_id) {
    if (device_id == nullptr) {
        LOG(ERROR) << " Invalid device id."; 

        return -1;
    }

    bool succeeded;
    QUERY_DEVICE_INFO(succeeded, device_id, CL_DEVICE_OPENCL_C_VERSION,  
                      PARAM_SIZE0, device_opencl_c_version_, true);
    if (device_opencl_c_version_.find("2.0") != string::npos) {
        opencl_version_ = 200;
    } 
    else if (device_opencl_c_version_.find("1.2") != string::npos) { 
        opencl_version_ = 120;
    } 
    else if (device_opencl_c_version_.find("1.1") != string::npos) { 
        opencl_version_ = 110;
    } 
    else {
       opencl_version_ = 100; 
    }

    return opencl_version_;
}

bool Device::getDeviceBasicInfos(const cl_device_id& device_id) {
    if (!device_detected_) {
        LOG(ERROR) << "device has not been detected."; 
        return false;
    }

    bool succeeded;
    QUERY_DEVICE_INFO(succeeded, device_id, CL_DEVICE_VENDOR, PARAM_SIZE0, 
                      device_vendor_, true);
    // std::transform(device_vendor_.begin(), device_vendor_.end(), device_vendor_.begin(), ::tolower);
    if (device_vendor_.find("QUALCOMM") != string::npos) {
        gpu_type_ = ADRENO_GPU;
    } 
    else if (device_vendor_.find("ARM") != string::npos) { // "mali" ?
        gpu_type_ = MALI_GPU;
    } 
    else {
       gpu_type_ = OTHER_GPU; 
    }

    QUERY_DEVICE_INFO(succeeded, device_id, CL_DEVICE_OPENCL_C_VERSION,  
                      PARAM_SIZE0, device_opencl_c_version_, true);
    if (device_opencl_c_version_.find("2.0") != string::npos) {
        opencl_version_ = 200;
    } 
    else if (device_opencl_c_version_.find("1.2") != string::npos) { 
        opencl_version_ = 120;
    } 
    else if (device_opencl_c_version_.find("1.1") != string::npos) { 
        opencl_version_ = 110;
    } 
    else {
       opencl_version_ = 100; 
    }

    std::string elements(PARAM_SIZE0);                   
    QUERY_DEVICE_INFO(succeeded, device_id, CL_DEVICE_MAX_COMPUTE_UNITS,  
                      sizeof(cl_uint), elements, false);
    compute_units_ = *((cl_uint*)elements.data());
    QUERY_DEVICE_INFO(succeeded, device_id, CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS, 
                      sizeof(cl_uint), elements, false);
    max_work_dim_ = *((cl_uint*)elements.data());
    QUERY_DEVICE_INFO(succeeded, device_id, CL_DEVICE_MAX_WORK_GROUP_SIZE, 
                      sizeof(size_t), elements, false);
    max_items_in_group_ = *((size_t*)elements.data());
    QUERY_DEVICE_INFO(succeeded, device_id, CL_DEVICE_MAX_WORK_ITEM_SIZES, 
                      sizeof(cl_uint3), elements, false);
    max_group_items_per_dim_ = *((cl_uint3*)elements.data());
    // QUERY_DEVICE_INFO(succeeded, device_id, CL_DEVICE_MAX_WORK_ITEM_SIZES, 
    //                   sizeof(size_t) * 3, elements, false);
    // max_group_items_per_dim_[0] = *((size_t*)elements.data());
    // max_group_items_per_dim_[1] = *((size_t*)elements.data() + 1);
    // max_group_items_per_dim_[2] = *((size_t*)elements.data() + 2);

    QUERY_DEVICE_INFO(succeeded, device_id, CL_DEVICE_MAX_MEM_ALLOC_SIZE, 
                      sizeof(cl_ulong), elements, false);
    max_mem_alloc_size_ = *((cl_ulong*)elements.data());
    QUERY_DEVICE_INFO(succeeded, device_id, CL_DEVICE_GLOBAL_MEM_SIZE, 
                      sizeof(cl_ulong), elements, false);
    global_mem_size_ = *((cl_ulong*)elements.data());

    QUERY_DEVICE_INFO(succeeded, device_id, CL_DEVICE_IMAGE_SUPPORT, sizeof(cl_bool), 
                      elements, false);
    image_supported_ = *((cl_bool*)elements.data());
    QUERY_DEVICE_INFO(succeeded, device_id, CL_DEVICE_IMAGE2D_MAX_WIDTH, 
                      sizeof(size_t), elements, false);
    image2d_max_width_ = *((size_t*)elements.data());    
    QUERY_DEVICE_INFO(succeeded, device_id, CL_DEVICE_IMAGE2D_MAX_HEIGHT, 
                      sizeof(size_t), elements, false);
    image2d_max_height_ = *((size_t*)elements.data());    
    QUERY_DEVICE_INFO(succeeded, device_id, CL_DEVICE_IMAGE3D_MAX_WIDTH, 
                      sizeof(size_t), elements, false);
    image3d_max_width_ = *((size_t*)elements.data());    
    QUERY_DEVICE_INFO(succeeded, device_id, CL_DEVICE_IMAGE3D_MAX_HEIGHT, 
                      sizeof(size_t), elements, false);
    image3d_max_height_ = *((size_t*)elements.data());    
    QUERY_DEVICE_INFO(succeeded, device_id, CL_DEVICE_IMAGE3D_MAX_DEPTH, 
                      sizeof(size_t), elements, false);
    image3d_max_depth_ = *((size_t*)elements.data());    
    QUERY_DEVICE_INFO(succeeded, device_id, CL_DEVICE_MAX_SAMPLERS, 
                      sizeof(cl_uint), elements, false);
    max_samplers_ = *((cl_uint*)elements.data());    

    QUERY_DEVICE_INFO(succeeded, device_id, CL_DEVICE_LOCAL_MEM_TYPE, 
                      sizeof(cl_device_local_mem_type), elements, false);
    local_mem_type_ = *((cl_device_local_mem_type*)elements.data());
    QUERY_DEVICE_INFO(succeeded, device_id, CL_DEVICE_LOCAL_MEM_SIZE, 
                      sizeof(cl_ulong), elements, false);
    local_mem_size_ = *((cl_ulong*)elements.data());

    return true;
}

bool Device::getDeviceBasicInfos() {
    bool succeeded = getDeviceBasicInfos(device_ids_[device_id_]);

    return succeeded;
}

bool Device::getDeviceThoroughInfos(const cl_device_id& device_id) {
    if (!device_detected_) {
        LOG(ERROR) << "device has not been detected."; 

        return false;
    }

    bool succeeded;
    QUERY_DEVICE_INFO(succeeded, device_id, CL_DEVICE_NAME, PARAM_SIZE0, 
                      device_name_, true);
    QUERY_DEVICE_INFO(succeeded, device_id, CL_DEVICE_PROFILE, PARAM_SIZE0, 
                      device_profile_, true);
    QUERY_DEVICE_INFO(succeeded, device_id, CL_DEVICE_VERSION, PARAM_SIZE0, 
                      device_version_, true);
    QUERY_DEVICE_INFO(succeeded, device_id, CL_DRIVER_VERSION, PARAM_SIZE0, 
                      driver_version_, true);
    QUERY_DEVICE_INFO(succeeded, device_id, CL_DEVICE_EXTENSIONS, PARAM_SIZE1, 
                      device_extensions_, true);

    QUERY_DEVICE_INFO(succeeded, device_id, CL_DEVICE_IL_VERSION, PARAM_SIZE0, 
                      device_il_version_, true);
    QUERY_DEVICE_INFO(succeeded, device_id, CL_DEVICE_BUILT_IN_KERNELS,  
                      PARAM_SIZE1, built_in_kernels_, true);

    std::string elements(PARAM_SIZE0);
    QUERY_DEVICE_INFO(succeeded, device_id, CL_DEVICE_NATIVE_VECTOR_WIDTH_CHAR, 
                      sizeof(cl_uint), elements, false);
    native_vector_width_char_ = *((cl_uint*)elements.data());
    QUERY_DEVICE_INFO(succeeded, device_id, CL_DEVICE_NATIVE_VECTOR_WIDTH_SHORT, 
                      sizeof(cl_uint), elements, false);
    native_vector_width_short_ = *((cl_uint*)elements.data());
    QUERY_DEVICE_INFO(succeeded, device_id, CL_DEVICE_NATIVE_VECTOR_WIDTH_INT, 
                      sizeof(cl_uint), elements, false);
    native_vector_width_int_ = *((cl_uint*)elements.data());
    QUERY_DEVICE_INFO(succeeded, device_id, CL_DEVICE_NATIVE_VECTOR_WIDTH_LONG, 
                      sizeof(cl_uint), elements, false);
    native_vector_width_long_ = *((cl_uint*)elements.data());
    QUERY_DEVICE_INFO(succeeded, device_id, CL_DEVICE_NATIVE_VECTOR_WIDTH_HALF, 
                      sizeof(cl_uint), elements, false);
    native_vector_width_half_ = *((cl_uint*)elements.data());
    QUERY_DEVICE_INFO(succeeded, device_id, CL_DEVICE_NATIVE_VECTOR_WIDTH_FLOAT, 
                      sizeof(cl_uint), elements, false);
    native_vector_width_float_ = *((cl_uint*)elements.data());
    QUERY_DEVICE_INFO(succeeded, device_id, 
                      CL_DEVICE_NATIVE_VECTOR_WIDTH_DOUBLE, sizeof(cl_uint), 
                      elements, false);
    native_vector_width_double_ = *((cl_uint*)elements.data());

    QUERY_DEVICE_INFO(succeeded, device_id, 
                      CL_DEVICE_PREFERRED_VECTOR_WIDTH_CHAR, sizeof(cl_uint), 
                      elements, false);
    preferred_vector_width_char_ = *((cl_uint*)elements.data());
    QUERY_DEVICE_INFO(succeeded, device_id, 
                      CL_DEVICE_PREFERRED_VECTOR_WIDTH_SHORT, sizeof(cl_uint), 
                      elements, false);
    preferred_vector_width_short_ = *((cl_uint*)elements.data());
    QUERY_DEVICE_INFO(succeeded, device_id, 
                      CL_DEVICE_PREFERRED_VECTOR_WIDTH_INT, sizeof(cl_uint), 
                      elements, false);
    preferred_vector_width_int_ = *((cl_uint*)elements.data());
    QUERY_DEVICE_INFO(succeeded, device_id, 
                      CL_DEVICE_PREFERRED_VECTOR_WIDTH_LONG, sizeof(cl_uint), 
                      elements, false);
    preferred_vector_width_long_ = *((cl_uint*)elements.data());
    QUERY_DEVICE_INFO(succeeded, device_id, 
                      CL_DEVICE_PREFERRED_VECTOR_WIDTH_HALF, sizeof(cl_uint), 
                      elements, false);
    preferred_vector_width_half_ = *((cl_uint*)elements.data());
    QUERY_DEVICE_INFO(succeeded, device_id, 
                      CL_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT, sizeof(cl_uint), 
                      elements, false);
    preferred_vector_width_float_ = *((cl_uint*)elements.data());
    QUERY_DEVICE_INFO(succeeded, device_id, 
                      CL_DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE, sizeof(cl_uint), 
                      elements, false);
    preferred_vector_width_double_ = *((cl_uint*)elements.data());

    QUERY_DEVICE_INFO(succeeded, device_id, CL_DEVICE_SINGLE_FP_CONFIG, 
                      sizeof(cl_device_fp_config), elements, false);
    single_fp_config_ = *((cl_device_fp_config*)elements.data());
    QUERY_DEVICE_INFO(succeeded, device_id, CL_DEVICE_DOUBLE_FP_CONFIG, 
                      sizeof(cl_device_fp_config), elements, false);
    double_fp_config_ = *((cl_device_fp_config*)elements.data());

    QUERY_DEVICE_INFO(succeeded, device_id, CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE, 
                      sizeof(cl_int), elements, false);
    global_mem_cacheline_size_ = *((cl_int*)elements.data());
    QUERY_DEVICE_INFO(succeeded, device_id, CL_DEVICE_GLOBAL_MEM_CACHE_SIZE, 
                      sizeof(cl_ulong), elements, false);
    global_mem_cache_size_ = *((cl_ulong*)elements.data());
    QUERY_DEVICE_INFO(succeeded, device_id, CL_DEVICE_GLOBAL_MEM_CACHE_TYPE, 
                      sizeof(cl_device_mem_cache_type), elements, false);
    global_mem_cache_type_ = *((cl_device_mem_cache_type*)elements.data());

    QUERY_DEVICE_INFO(succeeded, device_id, CL_DEVICE_MAX_READ_IMAGE_ARGS, 
                      sizeof(cl_int), elements, false);
    max_read_image_args_ = *((cl_int*)elements.data());
    QUERY_DEVICE_INFO(succeeded, device_id, CL_DEVICE_MAX_WRITE_IMAGE_ARGS, 
                      sizeof(cl_int), elements, false);
    max_write_image_args_ = *((cl_int*)elements.data());
    QUERY_DEVICE_INFO(succeeded, device_id, CL_DEVICE_MAX_READ_WRITE_IMAGE_ARGS, 
                      sizeof(cl_int), elements, false);
    max_read_write_image_args_ = *((cl_int*)elements.data());

    QUERY_DEVICE_INFO(succeeded, device_id, CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE, 
                      sizeof(cl_ulong), elements, false);
    constant_buffer_size_ = *((cl_ulong*)elements.data());
    QUERY_DEVICE_INFO(succeeded, device_id, CL_DEVICE_MAX_CONSTANT_ARGS, 
                      sizeof(cl_int), elements, false);
    max_constant_args_ = *((cl_int*)elements.data());
    QUERY_DEVICE_INFO(succeeded, device_id, CL_DEVICE_MAX_GLOBAL_VARIABLE_SIZE, 
                      sizeof(size_t), elements, false);
    max_global_variable_size_ = *((size_t*)elements.data());

    QUERY_DEVICE_INFO(succeeded, device_id, CL_DEVICE_MAX_CLOCK_FREQUENCY, 
                      sizeof(cl_uint), elements, false);
    max_clock_frequency_ = *((cl_uint*)elements.data());
    QUERY_DEVICE_INFO(succeeded, device_id, CL_DEVICE_ADDRESS_BITS, 
                      sizeof(cl_uint), elements, false);
    address_bits_ = *((cl_uint*)elements.data());
    QUERY_DEVICE_INFO(succeeded, device_id, CL_DEVICE_MAX_PARAMETER_SIZE, 
                      sizeof(size_t), elements, false);
    max_parameter_size_ = *((size_t*)elements.data());

    QUERY_DEVICE_INFO(succeeded, device_id, CL_DEVICE_ENDIAN_LITTLE, 
                      sizeof(cl_bool), elements, false);
    endian_little_ = *((cl_bool*)elements.data());

    QUERY_DEVICE_INFO(succeeded, device_id, CL_DEVICE_AVAILABLE, 
                      sizeof(cl_bool), elements, false);
    device_available_ = *((cl_bool*)elements.data());

    QUERY_DEVICE_INFO(succeeded, device_id, CL_DEVICE_COMPILER_AVAILABLE, 
                      sizeof(cl_bool), elements, false);
    compiler_abailable_ = *((cl_bool*)elements.data());

    QUERY_DEVICE_INFO(succeeded, device_id, CL_DEVICE_LINKER_AVAILABLE, 
                      sizeof(cl_bool), elements, false);
    linker_abailable_ = *((cl_bool*)elements.data());

    QUERY_DEVICE_INFO(succeeded, device_id, CL_DEVICE_QUEUE_ON_HOST_PROPERTIES, 
                      sizeof(cl_command_queue_properties), elements, false);
    host_queue_properties_ = *((cl_command_queue_properties*)elements.data());

    QUERY_DEVICE_INFO(succeeded, device_id, 
                      CL_DEVICE_QUEUE_ON_DEVICE_PROPERTIES, 
                      sizeof(cl_command_queue_properties), elements, false);
    device_queue_properties_ = *((cl_command_queue_properties*)elements.data());

    return true;
}

bool Device::getDeviceThoroughInfos() {
    bool succeeded = getDeviceThoroughInfos(device_ids_[device_id_]);

    return succeeded;
}

}}}    