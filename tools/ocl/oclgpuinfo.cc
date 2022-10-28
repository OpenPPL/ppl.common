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

#include "oclgpuinfo.h"

#include <string>
#include <iostream>
#include <iomanip>

namespace ppl { namespace common { namespace ocl {

#define WIDTH 50

#define FORMAT_ITEM_INFO0(info, value)                                         \
    meaning = std::string(info) + std::string(": ");                           \
    std::cout << "  " << std::setw(WIDTH) << meaning << value << std::endl;

#define FORMAT_ITEM_INFO1(info, io_flag, value)                                \
    meaning = std::string(info) + std::string(": ");                           \
    std::cout << "  " << std::setw(WIDTH) << meaning << io_flag << value       \
              << std::endl;

void OclGpuInfo::showGpuInformation(int platform_index, int device_index) {
    std::cout << "Detect and show GPU information with OpenCL..." << std::endl
              << std::endl;
    std::cout << std::left;

    std::string meaning;
    detectPlatforms();
    int platform_num = getPlatformNum();
    if (platform_num == 0) {
        std::cout << "Error: no valid OpenCL platform is detected."
                  << std::endl;
        return;
    }
    if (platform_index >= platform_num) {
        std::cout << "Error: Platform index should less than ." << platform_num
                  << std::endl;
        return;
    }
    std::cout << "Detected " << platform_num << " capable platforms."
              << std::endl;
    setPlatformIndex(platform_index);
    getPlatformInfos();
    std::cout << "Information about platform " << platform_index << ": "
              << std::endl;
    FORMAT_ITEM_INFO0("Platform name", platform_name_);
    FORMAT_ITEM_INFO0("Platform vendor", platform_vendor_);
    FORMAT_ITEM_INFO0("Platform profile", platform_profile_);
    FORMAT_ITEM_INFO0("Platform extensions", platform_extensions_);
    FORMAT_ITEM_INFO0("OpenCL version supported by implementation",
                      platform_version_);

    detectDevices();
    int device_num = getDeviceNum();
    if (device_num == 0) {
        std::cout << "Error: no valid OpenCL device is detected." << std::endl;
        return;
    }
    if (device_index >= device_num) {
        std::cout << "Error: Device index should less than ." << device_num
                  << std::endl;
        return;
    }
    std::cout << std::endl;
    std::cout << "Detected " << device_num << " capable devices." << std::endl;
    setDeviceIndex(device_index);
    getDeviceBasicInfos();
    getDeviceThoroughInfos();
    std::cout << "Information about device " << device_index << ": "
              << std::endl;
    FORMAT_ITEM_INFO0("Device name", device_name_);
    FORMAT_ITEM_INFO0("Device vendor", device_vendor_);
    FORMAT_ITEM_INFO0("Device profile", device_profile_);
    FORMAT_ITEM_INFO0("Driver version", driver_version_);
    FORMAT_ITEM_INFO0("OpenCL version supported by device", device_version_);
    FORMAT_ITEM_INFO0("OpenCL C version supported by compiler",
                      device_opencl_c_version_);
    FORMAT_ITEM_INFO0("Device extensions", device_extensions_);
#if CL_TARGET_OPENCL_VERSION >= 220
    FORMAT_ITEM_INFO0("Device il version", device_il_version_);
#endif
    FORMAT_ITEM_INFO0("Device built in kernels", built_in_kernels_);

    FORMAT_ITEM_INFO0("Max compute unites", compute_units_);
    FORMAT_ITEM_INFO0("Max work item dimensions", max_work_dim_);
    FORMAT_ITEM_INFO0("Max work items in a work group", max_items_in_group_);
    meaning = std::string("Max work items each dimension in a work group: ");
    std::cout << "  " << std::setw(WIDTH) << meaning;
    for (unsigned int i = 0; i < max_work_dim_; i++) {
        std::cout << max_group_items_per_dim_[i] << " ";
    }
    std::cout << std::endl;

    FORMAT_ITEM_INFO0("Native vector width(char)", native_vector_width_char_);
    FORMAT_ITEM_INFO0("Native vector width(short)", native_vector_width_short_);
    FORMAT_ITEM_INFO0("Native vector width(int)", native_vector_width_int_);
    FORMAT_ITEM_INFO0("Native vector width(long)", native_vector_width_long_);
    FORMAT_ITEM_INFO0("Native vector width(half)", native_vector_width_half_);
    FORMAT_ITEM_INFO0("Native vector width(float)", native_vector_width_float_);
    FORMAT_ITEM_INFO0("Native vector width(double)",
                      native_vector_width_double_);

    FORMAT_ITEM_INFO0("Preferred vector width(char)",
                      preferred_vector_width_char_);
    FORMAT_ITEM_INFO0("Preferred vector width(short)",
                      preferred_vector_width_short_);
    FORMAT_ITEM_INFO0("Preferred vector width(int)",
                      preferred_vector_width_int_);
    FORMAT_ITEM_INFO0("Preferred vector width(long)",
                      preferred_vector_width_long_);
    FORMAT_ITEM_INFO0("Preferred vector width(half)",
                      preferred_vector_width_half_);
    FORMAT_ITEM_INFO0("Preferred vector width(float)",
                      preferred_vector_width_float_);
    FORMAT_ITEM_INFO0("Preferred vector width(double)",
                      preferred_vector_width_double_);

    meaning = std::string("Single float point configuration: ");
    std::cout << "  " << std::setw(WIDTH) << meaning;
    if (single_fp_config_ & CL_FP_DENORM) {
        std::cout << "CL_FP_DENORM ";
    }
    if (single_fp_config_ & CL_FP_INF_NAN) {
        std::cout << "CL_FP_INF_NAN ";
    }
    if (single_fp_config_ & CL_FP_ROUND_TO_NEAREST) {
        std::cout << "CL_FP_ROUND_TO_NEAREST ";
    }
    if (single_fp_config_ & CL_FP_ROUND_TO_ZERO) {
        std::cout << "CL_FP_ROUND_TO_ZERO ";
    }
    if (single_fp_config_ & CL_FP_ROUND_TO_INF) {
        std::cout << "CL_FP_ROUND_TO_INF ";
    }
    if (single_fp_config_ & CL_FP_FMA) {
        std::cout << "CL_FP_FMA ";
    }
    if (single_fp_config_ & CL_FP_CORRECTLY_ROUNDED_DIVIDE_SQRT) {
        std::cout << "CL_FP_CORRECTLY_ROUNDED_DIVIDE_SQRT ";
    }
    if (single_fp_config_ & CL_FP_SOFT_FLOAT) {
        std::cout << "CL_FP_SOFT_FLOAT ";
    }
    std::cout << std::endl;

    meaning = std::string("Double float point configuration: ");
    std::cout << "  " << std::setw(WIDTH) << meaning;
    if (double_fp_config_ & CL_FP_DENORM) {
        std::cout << "CL_FP_DENORM ";
    }
    if (double_fp_config_ & CL_FP_INF_NAN) {
        std::cout << "CL_FP_INF_NAN ";
    }
    if (double_fp_config_ & CL_FP_ROUND_TO_NEAREST) {
        std::cout << "CL_FP_ROUND_TO_NEAREST ";
    }
    if (double_fp_config_ & CL_FP_ROUND_TO_ZERO) {
        std::cout << "CL_FP_ROUND_TO_ZERO ";
    }
    if (double_fp_config_ & CL_FP_ROUND_TO_INF) {
        std::cout << "CL_FP_ROUND_TO_INF ";
    }
    if (double_fp_config_ & CL_FP_FMA) {
        std::cout << "CL_FP_FMA ";
    }
    if (double_fp_config_ & CL_FP_SOFT_FLOAT) {
        std::cout << "CL_FP_SOFT_FLOAT ";
    }
    std::cout << std::endl;

    FORMAT_ITEM_INFO0("Max memory object allocation size in bytes",
                      max_mem_alloc_size_);
    FORMAT_ITEM_INFO0("Global memory size in bytes", global_mem_size_);
    FORMAT_ITEM_INFO0("Global memory cache line size in bytes",
                      global_mem_cacheline_size_);
    FORMAT_ITEM_INFO0("Global memory cache size in bytes",
                      global_mem_cache_size_);

    meaning = std::string("Global memory cache type: ");
    std::cout << "  " << std::setw(WIDTH) << meaning;
    if (global_mem_cache_type_ == CL_NONE) {
        std::cout << "CL_NONE";
    }
    if (global_mem_cache_type_ == CL_READ_ONLY_CACHE) {
        std::cout << "CL_READ_ONLY_CACHE";
    }
    if (global_mem_cache_type_ == CL_READ_WRITE_CACHE) {
        std::cout << "CL_READ_WRITE_CACHE";
    }
    std::cout << std::endl;

    FORMAT_ITEM_INFO1("Image memory support", std::boolalpha, image_supported_);
    FORMAT_ITEM_INFO0("Max read_only image arguments of a kernel",
                      max_read_image_args_);
    FORMAT_ITEM_INFO0("Max write_only image arguments of a kernel",
                      max_write_image_args_);
#if CL_TARGET_OPENCL_VERSION >= 200
    FORMAT_ITEM_INFO0("Max read/write only image arguments of a kernel",
                      max_read_write_image_args_);
#endif
    FORMAT_ITEM_INFO0("Max image1d or image2d width in pixels",
                      image2d_max_width_);
    FORMAT_ITEM_INFO0("Max image2d height in pixels", image2d_max_height_);
    FORMAT_ITEM_INFO0("Max image3d width in pixels", image3d_max_width_);
    FORMAT_ITEM_INFO0("Max image3d height in pixels", image3d_max_height_);
    FORMAT_ITEM_INFO0("Max image3d depth in pixels", image3d_max_depth_);
    FORMAT_ITEM_INFO0("Max image samplers used in a kernel", max_samplers_);

    FORMAT_ITEM_INFO0("Max constant buffer size", constant_buffer_size_);
    FORMAT_ITEM_INFO0("Max constant arguments in a kernel", max_constant_args_);
#if CL_TARGET_OPENCL_VERSION >= 200
    FORMAT_ITEM_INFO0("Max global variable size", max_global_variable_size_);
#endif
    // FORMAT_ITEM_INFO0("Local memory type", local_mem_type_);
    FORMAT_ITEM_INFO0("Local memory size in bytes", local_mem_size_);

    FORMAT_ITEM_INFO0("Device clock frequency in MHz", max_clock_frequency_);
    FORMAT_ITEM_INFO0("Global address space size in bits", address_bits_);
    FORMAT_ITEM_INFO0("Max size of kernel arguments in bytes",
                      max_parameter_size_);
    FORMAT_ITEM_INFO1("Little endian device", std::boolalpha,
                      (endian_little_ ? true : false));
    FORMAT_ITEM_INFO1("Device available", std::boolalpha,
                      (device_available_ ? true : false));
    FORMAT_ITEM_INFO1("Compiler available", std::boolalpha,
                      (compiler_abailable_ ? true : false));
    FORMAT_ITEM_INFO1("Linker available", std::boolalpha,
                      (linker_abailable_ ? true : false));

#if CL_TARGET_OPENCL_VERSION >= 200
    meaning = std::string("Properties of queue on host: ");
    std::cout << "  " << std::setw(WIDTH) << meaning;
    if (host_queue_properties_ & CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE) {
        std::cout << "CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE ";
    }
    if (host_queue_properties_ & CL_QUEUE_PROFILING_ENABLE) {
        std::cout << "CL_QUEUE_PROFILING_ENABLE ";
    }
    std::cout << std::endl;

    meaning = std::string("Properties of queue on device: ");
    std::cout << "  " << std::setw(WIDTH) << meaning;
    if (device_queue_properties_ & CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE) {
        std::cout << "CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE ";
    }
    if (device_queue_properties_ & CL_QUEUE_PROFILING_ENABLE) {
        std::cout << "CL_QUEUE_PROFILING_ENABLE ";
    }
    std::cout << std::endl;
#endif
}

}}}

int main(int argc, char **argv) {
    ppl::common::ocl::OclGpuInfo ocl_gpu_info;
    ocl_gpu_info.showGpuInformation(0, 0);

    return 0;
}
