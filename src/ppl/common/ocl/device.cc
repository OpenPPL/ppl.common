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

#include "ppl/common/log.h"

namespace ppl { namespace common { namespace ocl {

Device::Device() : platform_detected_(false), device_detected_(false), platform_index_(0), device_index_(0) {
    bool succeeded = detectAValidPlatformDevice();
    if (!succeeded)
        return;

    getDeviceBasicInfos();
}

Device::Device(int platform_index, int device_index)
    : platform_detected_(false), device_detected_(false), platform_index_(platform_index), device_index_(device_index) {
    bool succeeded = detectPlatforms();
    if (!succeeded)
        return;

    int number = getPlatformNum();
    if (number <= platform_index) {
        LOG(ERROR) << "The platform index must be less than the number of "
                   << "platforms.";
        return;
    }

    succeeded = detectDevices();
    if (!succeeded)
        return;

    number = getDeviceNum();
    if (number <= device_index) {
        LOG(ERROR) << "The device index must be less than the number of "
                   << "devices.";
        return;
    }

    getDeviceBasicInfos();
}

Device::~Device() {}

bool Device::detectPlatforms() {
    cl_int error_code;
    cl_uint num_platforms;
    error_code = clGetPlatformIDs(0, nullptr, &num_platforms);
    if (error_code != CL_SUCCESS) {
        LOG(ERROR) << "Call clGetPlatformIDs() failed with code: " << error_code;
        return false;
    }

    if (num_platforms <= 0) {
        LOG(ERROR) << "No platform is detected by clGetPlatformIDs().";
        return false;
    }

    platform_ids_.resize(num_platforms);
    error_code = clGetPlatformIDs(num_platforms, platform_ids_.data(), nullptr);
    if (error_code != CL_SUCCESS) {
        LOG(ERROR) << "Call clGetPlatformIDs() failed with code: " << error_code;
        return false;
    }

    platform_detected_ = true;

    return true;
}

int Device::getPlatformNum() const {
    if (platform_detected_) {
        return platform_ids_.size();
    } else {
        return 0;
    }
}

void Device::setPlatformIndex(int index) {
    platform_index_ = index;
}

bool Device::detectDevices() {
    if (device_ids_.size() != 0) {
        device_ids_.clear();
    }

    if (!platform_detected_ || platform_index_ >= (int)platform_ids_.size()) {
        LOG(ERROR) << "Invalid platform id.";
        return false;
    }

    cl_int error_code;
    cl_uint num_devices;
    error_code = clGetDeviceIDs(platform_ids_[platform_index_], CL_DEVICE_TYPE_GPU, 0, nullptr, &num_devices);
    if (error_code != CL_SUCCESS) {
        LOG(ERROR) << "Call clGetDeviceIDs() failed with code: " << error_code;
        return false;
    }

    if (num_devices <= 0) {
        LOG(ERROR) << "No device is detected by clGetDeviceIDs().";
        return error_code;
    }

    device_ids_.resize(num_devices);
    error_code =
        clGetDeviceIDs(platform_ids_[platform_index_], CL_DEVICE_TYPE_GPU, num_devices, device_ids_.data(), nullptr);
    if (error_code != CL_SUCCESS) {
        LOG(ERROR) << "Call clGetDeviceIDs() failed with code: " << error_code;
        return false;
    }

    device_detected_ = true;

    return true;
}

int Device::getDeviceNum() const {
    if (device_detected_) {
        return device_ids_.size();
    } else {
        return 0;
    }
}

void Device::setDeviceIndex(int index) {
    device_index_ = index;
}

bool Device::detectAValidPlatformDevice() {
    bool succeeded = detectPlatforms();
    if (!succeeded)
        return false;

    int platforms = getPlatformNum();
    if (platforms <= 0) {
        LOG(ERROR) << "There is no valid platform.";
        return false;
    }

    for (int platform_index = 0; platform_index < platforms; platform_index++) {
        setPlatformIndex(platform_index);
        succeeded = detectDevices();
        if (!succeeded)
            return false;

        int devices = getDeviceNum();
        if (devices == 0)
            continue;
        for (int device_index = 0; device_index < devices; device_index++) {
            cl_device_id device_id = device_ids_[device_index];
            if (device_id != nullptr) {
                platform_index_ = platform_index;
                device_index_ = device_index;
                return true;
            }
        }
    }

    LOG(ERROR) << "There is no valid platform and device.";
    return false;
}

cl_platform_id Device::getPlatformId() {
    if (platform_detected_) {
        return platform_ids_[platform_index_];
    } else {
        return nullptr;
    }
}

cl_platform_id Device::getPlatformId(int index) {
    if (platform_detected_) {
        return platform_ids_[index];
    } else {
        return nullptr;
    }
}

cl_device_id Device::getDeviceId() {
    if (device_detected_) {
        return device_ids_[device_index_];
    } else {
        return nullptr;
    }
}

cl_device_id Device::getDeviceId(int index) {
    if (device_detected_) {
        return device_ids_[index];
    } else {
        return nullptr;
    }
}

bool Device::queryPlatformInfo(const cl_platform_id& platform_id, cl_platform_info param_name,
                               std::string& param_value) {
    if (platform_id == nullptr) {
        LOG(ERROR) << "Invalid platform id.";
        return false;
    }

    cl_int error_code;
    size_t returned_size;
    error_code = clGetPlatformInfo(platform_id, param_name, 0, nullptr, &returned_size);
    if (error_code != CL_SUCCESS) {
        LOG(ERROR) << "Call clGetPlatformInfo() failed with code: " << error_code;
        return error_code;
    }

    if (returned_size <= 0) {
        LOG(ERROR) << "No information is detected by clGetPlatformInfo().";
        return error_code;
    }

    param_value.resize(returned_size);
    error_code = clGetPlatformInfo(platform_id, param_name, returned_size, (void*)param_value.data(), nullptr);
    if (error_code != CL_SUCCESS) {
        LOG(ERROR) << "Call clGetPlatformInfo() failed with code: " << error_code;
        return error_code;
    }

    return true;
}

bool Device::getPlatformInfos() {
    if (!platform_detected_) {
        LOG(ERROR) << "platforms has not been detected.";
        return false;
    }

    bool succeeded;
    succeeded = queryPlatformInfo(platform_ids_[platform_index_], CL_PLATFORM_NAME, platform_name_);
    if (!succeeded) {
        return false;
    }

    succeeded = queryPlatformInfo(platform_ids_[platform_index_], CL_PLATFORM_VENDOR, platform_vendor_);
    if (!succeeded) {
        return false;
    }

    succeeded = queryPlatformInfo(platform_ids_[platform_index_], CL_PLATFORM_PROFILE, platform_profile_);
    if (!succeeded) {
        return false;
    }

    succeeded = queryPlatformInfo(platform_ids_[platform_index_], CL_PLATFORM_VERSION, platform_version_);
    if (!succeeded) {
        return false;
    }

    succeeded = queryPlatformInfo(platform_ids_[platform_index_], CL_PLATFORM_EXTENSIONS, platform_extensions_);
    if (!succeeded) {
        return false;
    }

    return true;
}

bool Device::queryDeviceInfo(const cl_device_id& device_id, cl_device_info param_name, std::string& param_value,
                             bool scaling) {
    if (device_id == nullptr) {
        LOG(ERROR) << "Invalid device id.";
        return false;
    }

    cl_int error_code;
    size_t returned_size;
    error_code = clGetDeviceInfo(device_id, param_name, 0, nullptr, &returned_size);
    if (error_code != CL_SUCCESS) {
        LOG(ERROR) << "Call clGetDeviceInfo() failed with code: " << error_code;
        return false;
    }

    if (returned_size <= 0) {
        LOG(ERROR) << "No information is detected by clGetDeviceInfo().";
        return error_code;
    }

    param_value.resize(returned_size);
    error_code = clGetDeviceInfo(device_id, param_name, returned_size, (void*)param_value.data(), nullptr);
    if (error_code != CL_SUCCESS) {
        LOG(ERROR) << "Call clGetDeviceInfo() failed with code: " << error_code;
        return false;
    }

    return true;
}

GpuTypes Device::checkGpuType(const cl_device_id& device_id) {
    if (device_id == nullptr) {
        LOG(ERROR) << "Invalid device id.";
        return INVALID_GPU;
    }

    bool succeeded = queryDeviceInfo(device_id, CL_DEVICE_VENDOR, device_vendor_, true);
    if (!succeeded) {
        return INVALID_GPU;
    }
    if (device_vendor_.find("QUALCOMM") != std::string::npos) {
        gpu_type_ = ADRENO_GPU;
    } else if (device_vendor_.find("ARM") != std::string::npos) {
        gpu_type_ = MALI_GPU;
    } else if (device_vendor_.find("NVIDIA") != std::string::npos) {
        gpu_type_ = NVIDIA_GPU;
    } else {
        gpu_type_ = OTHER_GPU;
    }

    return gpu_type_;
}

int Device::checkOpenCLVersion(const cl_device_id& device_id) {
    if (device_id == nullptr) {
        LOG(ERROR) << "Invalid device id.";
        return -1;
    }

    bool succeeded = queryDeviceInfo(device_id, CL_DEVICE_OPENCL_C_VERSION, device_opencl_c_version_, true);
    if (!succeeded) {
        return -1;
    }

    if (device_opencl_c_version_.find("3.0") != std::string::npos) {
        opencl_version_ = 300;
    } else if (device_opencl_c_version_.find("2.2") != std::string::npos) {
        opencl_version_ = 220;
    } else if (device_opencl_c_version_.find("2.0") != std::string::npos) {
        opencl_version_ = 200;
    } else if (device_opencl_c_version_.find("1.2") != std::string::npos) {
        opencl_version_ = 120;
    } else if (device_opencl_c_version_.find("1.1") != std::string::npos) {
        opencl_version_ = 110;
    } else {
        opencl_version_ = 100;
    }

    return opencl_version_;
}

#define QUERY_DEVICE_INFO(succeeded, device_id, param_name, param_value, scaling) \
    succeeded = queryDeviceInfo(device_id, param_name, param_value, scaling);     \
    if (!succeeded) {                                                             \
        return false;                                                             \
    }

bool Device::getDeviceBasicInfos(const cl_device_id& device_id) {
    if (device_id == nullptr) {
        LOG(ERROR) << "Invalid device id.";
        return false;
    }

    bool succeeded;
    QUERY_DEVICE_INFO(succeeded, device_id, CL_DEVICE_NAME, device_name_, true);
    QUERY_DEVICE_INFO(succeeded, device_id, CL_DEVICE_VENDOR, device_vendor_, true);
    if (device_vendor_.find("QUALCOMM") != std::string::npos) {
        gpu_type_ = ADRENO_GPU;
    } else if (device_vendor_.find("ARM") != std::string::npos) {
        gpu_type_ = MALI_GPU;
    } else if (device_vendor_.find("NVIDIA") != std::string::npos) {
        gpu_type_ = NVIDIA_GPU;
    } else {
        gpu_type_ = OTHER_GPU;
    }

    QUERY_DEVICE_INFO(succeeded, device_id, CL_DEVICE_OPENCL_C_VERSION, device_opencl_c_version_, true);
    if (device_opencl_c_version_.find("3.0") != std::string::npos) {
        opencl_version_ = 300;
    } else if (device_opencl_c_version_.find("2.2") != std::string::npos) {
        opencl_version_ = 220;
    } else if (device_opencl_c_version_.find("2.0") != std::string::npos) {
        opencl_version_ = 200;
    } else if (device_opencl_c_version_.find("1.2") != std::string::npos) {
        opencl_version_ = 120;
    } else if (device_opencl_c_version_.find("1.1") != std::string::npos) {
        opencl_version_ = 110;
    } else {
        opencl_version_ = 100;
    }

    std::string elements;
    elements.resize(PARAM_SIZE);
    QUERY_DEVICE_INFO(succeeded, device_id, CL_DEVICE_MAX_COMPUTE_UNITS, elements, false);
    compute_units_ = *((cl_uint*)elements.data());
    QUERY_DEVICE_INFO(succeeded, device_id, CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS, elements, false);
    max_work_dim_ = *((cl_uint*)elements.data());
    QUERY_DEVICE_INFO(succeeded, device_id, CL_DEVICE_MAX_WORK_GROUP_SIZE, elements, false);
    max_items_in_group_ = *((size_t*)elements.data());
    QUERY_DEVICE_INFO(succeeded, device_id, CL_DEVICE_MAX_WORK_ITEM_SIZES, elements, false);
    size_t* pointer = (size_t*)elements.data();
    for (unsigned int i = 0; i < max_work_dim_; i++) {
        max_group_items_per_dim_.push_back(*(pointer++));
    }

    QUERY_DEVICE_INFO(succeeded, device_id, CL_DEVICE_MAX_MEM_ALLOC_SIZE, elements, false);
    max_mem_alloc_size_ = *((cl_ulong*)elements.data());
    QUERY_DEVICE_INFO(succeeded, device_id, CL_DEVICE_GLOBAL_MEM_SIZE, elements, false);
    global_mem_size_ = *((cl_ulong*)elements.data());

    QUERY_DEVICE_INFO(succeeded, device_id, CL_DEVICE_IMAGE_SUPPORT, elements, false);
    image_supported_ = *((cl_bool*)elements.data());
    QUERY_DEVICE_INFO(succeeded, device_id, CL_DEVICE_IMAGE2D_MAX_WIDTH, elements, false);
    image2d_max_width_ = *((size_t*)elements.data());
    QUERY_DEVICE_INFO(succeeded, device_id, CL_DEVICE_IMAGE2D_MAX_HEIGHT, elements, false);
    image2d_max_height_ = *((size_t*)elements.data());
    QUERY_DEVICE_INFO(succeeded, device_id, CL_DEVICE_IMAGE3D_MAX_WIDTH, elements, false);
    image3d_max_width_ = *((size_t*)elements.data());
    QUERY_DEVICE_INFO(succeeded, device_id, CL_DEVICE_IMAGE3D_MAX_HEIGHT, elements, false);
    image3d_max_height_ = *((size_t*)elements.data());
    QUERY_DEVICE_INFO(succeeded, device_id, CL_DEVICE_IMAGE3D_MAX_DEPTH, elements, false);
    image3d_max_depth_ = *((size_t*)elements.data());
    QUERY_DEVICE_INFO(succeeded, device_id, CL_DEVICE_MAX_SAMPLERS, elements, false);
    max_samplers_ = *((cl_uint*)elements.data());

    QUERY_DEVICE_INFO(succeeded, device_id, CL_DEVICE_LOCAL_MEM_TYPE, elements, false);
    local_mem_type_ = *((cl_device_local_mem_type*)elements.data());
    QUERY_DEVICE_INFO(succeeded, device_id, CL_DEVICE_LOCAL_MEM_SIZE, elements, false);
    local_mem_size_ = *((cl_ulong*)elements.data());

    return true;
}

bool Device::getDeviceBasicInfos() {
    if (!device_detected_) {
        LOG(ERROR) << "device has not been detected.";
        return false;
    }

    bool succeeded = getDeviceBasicInfos(device_ids_[device_index_]);

    return succeeded;
}

bool Device::getDeviceThoroughInfos(const cl_device_id& device_id) {
    if (device_id == nullptr) {
        LOG(ERROR) << "Invalid device id.";
        return false;
    }

    bool succeeded;
    QUERY_DEVICE_INFO(succeeded, device_id, CL_DEVICE_NAME, device_name_, true);
    QUERY_DEVICE_INFO(succeeded, device_id, CL_DEVICE_PROFILE, device_profile_, true);
    QUERY_DEVICE_INFO(succeeded, device_id, CL_DEVICE_VERSION, device_version_, true);
    QUERY_DEVICE_INFO(succeeded, device_id, CL_DRIVER_VERSION, driver_version_, true);
    QUERY_DEVICE_INFO(succeeded, device_id, CL_DEVICE_EXTENSIONS, device_extensions_, true);

#if CL_TARGET_OPENCL_VERSION >= 210
    QUERY_DEVICE_INFO(succeeded, device_id, CL_DEVICE_IL_VERSION, device_il_version_, true);
#endif
    QUERY_DEVICE_INFO(succeeded, device_id, CL_DEVICE_BUILT_IN_KERNELS, built_in_kernels_, true);

    std::string elements;
    elements.resize(PARAM_SIZE);
    QUERY_DEVICE_INFO(succeeded, device_id, CL_DEVICE_NATIVE_VECTOR_WIDTH_CHAR, elements, false);
    native_vector_width_char_ = *((cl_uint*)elements.data());
    QUERY_DEVICE_INFO(succeeded, device_id, CL_DEVICE_NATIVE_VECTOR_WIDTH_SHORT, elements, false);
    native_vector_width_short_ = *((cl_uint*)elements.data());
    QUERY_DEVICE_INFO(succeeded, device_id, CL_DEVICE_NATIVE_VECTOR_WIDTH_INT, elements, false);
    native_vector_width_int_ = *((cl_uint*)elements.data());
    QUERY_DEVICE_INFO(succeeded, device_id, CL_DEVICE_NATIVE_VECTOR_WIDTH_LONG, elements, false);
    native_vector_width_long_ = *((cl_uint*)elements.data());
    QUERY_DEVICE_INFO(succeeded, device_id, CL_DEVICE_NATIVE_VECTOR_WIDTH_HALF, elements, false);
    native_vector_width_half_ = *((cl_uint*)elements.data());
    QUERY_DEVICE_INFO(succeeded, device_id, CL_DEVICE_NATIVE_VECTOR_WIDTH_FLOAT, elements, false);
    native_vector_width_float_ = *((cl_uint*)elements.data());
    QUERY_DEVICE_INFO(succeeded, device_id, CL_DEVICE_NATIVE_VECTOR_WIDTH_DOUBLE, elements, false);
    native_vector_width_double_ = *((cl_uint*)elements.data());

    QUERY_DEVICE_INFO(succeeded, device_id, CL_DEVICE_PREFERRED_VECTOR_WIDTH_CHAR, elements, false);
    preferred_vector_width_char_ = *((cl_uint*)elements.data());
    QUERY_DEVICE_INFO(succeeded, device_id, CL_DEVICE_PREFERRED_VECTOR_WIDTH_SHORT, elements, false);
    preferred_vector_width_short_ = *((cl_uint*)elements.data());
    QUERY_DEVICE_INFO(succeeded, device_id, CL_DEVICE_PREFERRED_VECTOR_WIDTH_INT, elements, false);
    preferred_vector_width_int_ = *((cl_uint*)elements.data());
    QUERY_DEVICE_INFO(succeeded, device_id, CL_DEVICE_PREFERRED_VECTOR_WIDTH_LONG, elements, false);
    preferred_vector_width_long_ = *((cl_uint*)elements.data());
    QUERY_DEVICE_INFO(succeeded, device_id, CL_DEVICE_PREFERRED_VECTOR_WIDTH_HALF, elements, false);
    preferred_vector_width_half_ = *((cl_uint*)elements.data());
    QUERY_DEVICE_INFO(succeeded, device_id, CL_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT, elements, false);
    preferred_vector_width_float_ = *((cl_uint*)elements.data());
    QUERY_DEVICE_INFO(succeeded, device_id, CL_DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE, elements, false);
    preferred_vector_width_double_ = *((cl_uint*)elements.data());

    QUERY_DEVICE_INFO(succeeded, device_id, CL_DEVICE_SINGLE_FP_CONFIG, elements, false);
    single_fp_config_ = *((cl_device_fp_config*)elements.data());
    QUERY_DEVICE_INFO(succeeded, device_id, CL_DEVICE_DOUBLE_FP_CONFIG, elements, false);
    double_fp_config_ = *((cl_device_fp_config*)elements.data());

    QUERY_DEVICE_INFO(succeeded, device_id, CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE, elements, false);
    global_mem_cacheline_size_ = *((cl_int*)elements.data());
    QUERY_DEVICE_INFO(succeeded, device_id, CL_DEVICE_GLOBAL_MEM_CACHE_SIZE, elements, false);
    global_mem_cache_size_ = *((cl_ulong*)elements.data());
    QUERY_DEVICE_INFO(succeeded, device_id, CL_DEVICE_GLOBAL_MEM_CACHE_TYPE, elements, false);
    global_mem_cache_type_ = *((cl_device_mem_cache_type*)elements.data());

    QUERY_DEVICE_INFO(succeeded, device_id, CL_DEVICE_MAX_READ_IMAGE_ARGS, elements, false);
    max_read_image_args_ = *((cl_int*)elements.data());
    QUERY_DEVICE_INFO(succeeded, device_id, CL_DEVICE_MAX_WRITE_IMAGE_ARGS, elements, false);
    max_write_image_args_ = *((cl_int*)elements.data());
#if CL_TARGET_OPENCL_VERSION >= 200
    QUERY_DEVICE_INFO(succeeded, device_id, CL_DEVICE_MAX_READ_WRITE_IMAGE_ARGS, elements, false);
    max_read_write_image_args_ = *((cl_int*)elements.data());
#endif

    QUERY_DEVICE_INFO(succeeded, device_id, CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE, elements, false);
    constant_buffer_size_ = *((cl_ulong*)elements.data());
    QUERY_DEVICE_INFO(succeeded, device_id, CL_DEVICE_MAX_CONSTANT_ARGS, elements, false);
#if CL_TARGET_OPENCL_VERSION >= 200
    max_constant_args_ = *((cl_int*)elements.data());
    QUERY_DEVICE_INFO(succeeded, device_id, CL_DEVICE_MAX_GLOBAL_VARIABLE_SIZE, elements, false);
    max_global_variable_size_ = *((size_t*)elements.data());
#endif

    QUERY_DEVICE_INFO(succeeded, device_id, CL_DEVICE_MAX_CLOCK_FREQUENCY, elements, false);
    max_clock_frequency_ = *((cl_uint*)elements.data());
    QUERY_DEVICE_INFO(succeeded, device_id, CL_DEVICE_ADDRESS_BITS, elements, false);
    address_bits_ = *((cl_uint*)elements.data());
    QUERY_DEVICE_INFO(succeeded, device_id, CL_DEVICE_MAX_PARAMETER_SIZE, elements, false);
    max_parameter_size_ = *((size_t*)elements.data());

    QUERY_DEVICE_INFO(succeeded, device_id, CL_DEVICE_ENDIAN_LITTLE, elements, false);
    endian_little_ = *((cl_bool*)elements.data());

    QUERY_DEVICE_INFO(succeeded, device_id, CL_DEVICE_AVAILABLE, elements, false);
    device_available_ = *((cl_bool*)elements.data());

    QUERY_DEVICE_INFO(succeeded, device_id, CL_DEVICE_COMPILER_AVAILABLE, elements, false);
    compiler_abailable_ = *((cl_bool*)elements.data());

    QUERY_DEVICE_INFO(succeeded, device_id, CL_DEVICE_LINKER_AVAILABLE, elements, false);
    linker_abailable_ = *((cl_bool*)elements.data());

#if CL_TARGET_OPENCL_VERSION >= 200
    QUERY_DEVICE_INFO(succeeded, device_id, CL_DEVICE_QUEUE_ON_HOST_PROPERTIES, elements, false);
    host_queue_properties_ = *((cl_command_queue_properties*)elements.data());

    QUERY_DEVICE_INFO(succeeded, device_id, CL_DEVICE_QUEUE_ON_DEVICE_PROPERTIES, elements, false);
    device_queue_properties_ = *((cl_command_queue_properties*)elements.data());

    QUERY_DEVICE_INFO(succeeded, device_id, CL_DEVICE_SVM_CAPABILITIES, elements, false);
    svm_capabilities_ = *((cl_device_svm_capabilities*)elements.data());
#endif

    return true;
}

bool Device::getDeviceThoroughInfos() {
    if (!device_detected_) {
        LOG(ERROR) << "device has not been detected.";
        return false;
    }

    bool succeeded = getDeviceThoroughInfos(device_ids_[device_index_]);

    return succeeded;
}

static Device* shared_device;

void createSharedDevice() {
    static Device device;

    shared_device = &device;
}

void createSharedDevice(int platform_index, int device_index) {
    static Device device(platform_index, device_index);

    shared_device = &device;
}

Device* getSharedDevice() {
    return shared_device;
}

size_t GetMaxImageWidth() {
    createSharedDevice();
    return shared_device->getMaxImageWidth();
}

size_t GetMaxImageHeight() {
    createSharedDevice();
    return shared_device->getMaxImageHeight();
}

size_t GetMaxImageDepth() {
    createSharedDevice();
    return shared_device->getMaxImageDepth();
}
}}} // namespace ppl::common::ocl
