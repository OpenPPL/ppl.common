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

#include "kernelbinariesmanager.h"
#include "runkernel.h"

#include <cstdio>
#include <cstring>

#include "CL/cl.h"

#include "ppl/common/log.h"

namespace ppl { namespace common { namespace ocl {

KernelBinariesManager::KernelBinariesManager()
    : fp_(nullptr), kernel_offset_(0), kernel_count_(0), function_offset_(0), function_count_(0), is_working_(false) {}

KernelBinariesManager::~KernelBinariesManager() {
    if (fp_ != nullptr) {
        fclose(fp_);
    }
}

bool KernelBinariesManager::prepareManager(BinariesManagerStatus status) {
    if (status == BINARIES_COMPILE) {
        fp_ = fopen(BINARIES_FILE, "wb");
        if (fp_ == nullptr) {
            LOG(ERROR) << "Failed to open the file: " << BINARIES_FILE;
            return false;
        }

        kernel_offset_ = sizeof(uint32_t) * 4;
        function_offset_ = kernel_offset_;
        size_t written_count;
        written_count = fwrite(&kernel_offset_, sizeof(uint32_t), 1, fp_);
        if (written_count != 1) {
            LOG(ERROR) << "Error in writing kernel info offset, written count: " << written_count
                       << ", writing count: " << 1;
            return false;
        }

        written_count = fwrite(&kernel_count_, sizeof(uint32_t), 1, fp_);
        if (written_count != 1) {
            LOG(ERROR) << "Error in writing kernel count, written count: " << written_count << ", writing count: " << 1;
            return false;
        }

        written_count = fwrite(&function_offset_, sizeof(uint32_t), 1, fp_);
        if (written_count != 1) {
            LOG(ERROR) << "Error in writing function info offset, written "
                       << "count: " << written_count << ", writing count: " << 1;
            return false;
        }

        written_count = fwrite(&function_count_, sizeof(uint32_t), 1, fp_);
        if (written_count != 1) {
            LOG(ERROR) << "Error in writing function count, written count: " << written_count
                       << ", writing count: " << 1;
            return false;
        }
        is_working_ = true;

        return true;
    } else if (status == BINARIES_RETRIEVE) {
        fp_ = fopen(BINARIES_FILE, "rb");
        if (fp_ == nullptr) {
            LOG(ERROR) << "Failed to open the file: " << BINARIES_FILE;
            return false;
        }

        size_t read_count;
        read_count = fread(&kernel_offset_, sizeof(uint32_t), 1, fp_);
        if (read_count != 1) {
            LOG(ERROR) << "Error in reading kernel info offset, read count: " << read_count << ", reading count: " << 1;
            return false;
        }

        read_count = fread(&kernel_count_, sizeof(uint32_t), 1, fp_);
        if (read_count != 1) {
            LOG(ERROR) << "Error in reading kernel count, read count: " << read_count << ", reading count: " << 1;
            return false;
        }

        read_count = fread(&function_offset_, sizeof(uint32_t), 1, fp_);
        if (read_count != 1) {
            LOG(ERROR) << "Error in reading function info offset, read count: " << read_count
                       << ", reading count: " << 1;
            return false;
        }

        read_count = fread(&function_count_, sizeof(uint32_t), 1, fp_);
        if (read_count != 1) {
            LOG(ERROR) << "Error in reading function count, read count: " << read_count << ", reading count: " << 1;
            return false;
        }
        is_working_ = true;

        return true;
    } else {
        return false;
    }
}

bool KernelBinariesManager::buildFunctionFromSource(FrameChain* frame_chain) {
    if (frame_chain == nullptr) {
        LOG(ERROR) << "The opencl frame chain is invalid.";
        return false;
    }

    const char* source_str = frame_chain->getCodeString();
    size_t kernel_length = strlen(source_str);
    if (source_str == nullptr || kernel_length == 0) {
        LOG(ERROR) << "The function name is invalid.";
        return false;
    }

    cl_int error_code;
    cl_program program;
    cl_context context = frame_chain->getContext();
    program = clCreateProgramWithSource(context, 1, &source_str, &kernel_length, &error_code);
    if (error_code != CL_SUCCESS) {
        LOG(ERROR) << "Call clCreateProgramWithSource() failed with code: " << error_code;
        return false;
    }
    frame_chain->setProgram(program);

    cl_device_id device_id = frame_chain->getDeviceId();
    std::string build_options = frame_chain->getCompileOptions();
    error_code = clBuildProgram(program, 1, &device_id, build_options.c_str(), nullptr, nullptr);
    if (error_code != CL_SUCCESS) {
        size_t log_length;
        cl_int code = clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, 0, nullptr, &log_length);
        if (code != CL_SUCCESS) {
            LOG(ERROR) << "Call clGetProgramBuildInfo() failed with code: " << code;
            return false;
        }

        std::string log_buffer((int)log_length, '0');
        code = clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, log_length, (char*)log_buffer.data(),
                                     nullptr);
        if (code != CL_SUCCESS) {
            LOG(ERROR) << "Call clGetProgramBuildInfo() failed with code: " << code;
            return false;
        }
        LOG(ERROR) << "Call clBuildProgram() failed with code: " << error_code;
        LOG(ERROR) << "clBuildProgram() log: " << log_buffer;

        return false;
    }

    return true;
}

bool KernelBinariesManager::storeFunctionBinaries(FrameChain* frame_chain) {
    if (frame_chain == nullptr) {
        LOG(ERROR) << "The opencl frame chain is invalid.";
        return false;
    }

    cl_int error_code;
    std::string project_name = frame_chain->getProjectName();
    std::string function_name = frame_chain->getFunctionName();
    std::lock_guard<std::mutex> lock_guard(locker_);
    auto& function2kernelinfo = function2binariesinfo_[project_name];
    auto iter = function2kernelinfo.find(function_name);
    if (iter != function2kernelinfo.end()) {
        LOG(INFO) << function_name << " has already been build and stored.";
        return true;
    }

    cl_program program = frame_chain->getProgram();
    size_t binaries_size;
    size_t returned_size;
    error_code = clGetProgramInfo(program, CL_PROGRAM_BINARY_SIZES, sizeof(size_t), &binaries_size, &returned_size);
    if (error_code != CL_SUCCESS) {
        LOG(ERROR) << "Call clGetProgramInfo() failed with code: " << error_code;
        return false;
    }

    unsigned char** program_binaries = new unsigned char*[1];
    program_binaries[0] = new unsigned char[binaries_size];
    error_code =
        clGetProgramInfo(program, CL_PROGRAM_BINARIES, sizeof(unsigned char**), program_binaries, &returned_size);
    if (error_code != CL_SUCCESS) {
        LOG(ERROR) << "Call clGetProgramInfo() failed with code: " << error_code;
        return false;
    }

    size_t written_count = fwrite(program_binaries[0], 1, binaries_size, fp_);
    if (written_count != binaries_size) {
        fseek(fp_, 0 - written_count, SEEK_CUR);
        LOG(ERROR) << "Error in writing file, written bytes: " << written_count << ", binary size: " << binaries_size;
        return false;
    }

    std::vector<std::string> kernel_names;
    getKernelNames(program, kernel_names);
    for (size_t i = 0; i < kernel_names.size(); i++) {
        auto iter = kernel2function_.find(kernel_names[i]);
        if (iter != kernel2function_.end()) {
            LOG(ERROR) << kernel_names[i] << " has already been in " << BINARIES_FILE << ", please rename the kernel.";
            return false;
        }
        kernel2function_[kernel_names[i]] = make_pair(project_name, function_name);
    }

    KernelBinaryInfo binaries_info;
    binaries_info.address_offset = function_offset_;
    binaries_info.binaries_size = binaries_size;
    function2kernelinfo[function_name] = binaries_info;

    function_offset_ += binaries_size;
    function_count_++;

    delete[](program_binaries[0]);
    delete[] program_binaries;

    return true;
}

bool KernelBinariesManager::storeMapstoFile() {
    Kernel2FunctionItem kernel_storage;
    Function2BinariesItem function_storage;
    std::string project_name;
    std::string function_name;
    std::string kernel_name;
    size_t written_count;
    size_t item_size0 = sizeof(Kernel2FunctionItem);
    size_t item_size1 = sizeof(Function2BinariesItem);

    kernel_offset_ = function_offset_;
    std::lock_guard<std::mutex> lock_guard(locker_);
    for (const auto& kernel_item : kernel2function_) {
        kernel_name = kernel_item.first;
        if (kernel_name.size() + 1 > KERNEL_LENGTH) {
            fseek(fp_, kernel_offset_, SEEK_SET);
            LOG(ERROR) << "kernel name is too long, please reset "
                       << "KERNEL_LENGTH.";
            return false;
        }
        strcpy(kernel_storage.kernel_name, kernel_name.c_str());

        project_name = kernel_item.second.first;
        if (project_name.size() + 1 > PROJECT_LENGTH) {
            fseek(fp_, kernel_offset_, SEEK_SET);
            LOG(ERROR) << "project name is too long, please reset "
                       << "PROJECT_LENGTH.";
            return false;
        }
        strcpy(kernel_storage.project_name, project_name.c_str());

        function_name = kernel_item.second.second;
        if (function_name.size() + 1 > FUNCTION_LENGTH) {
            fseek(fp_, kernel_offset_, SEEK_SET);
            LOG(ERROR) << "project name is too long, please reset "
                       << "FUNCTION_LENGTH.";
            return false;
        }
        strcpy(kernel_storage.function_name, function_name.c_str());

        written_count = fwrite(&kernel_storage, item_size0, 1, fp_);
        if (written_count != 1) {
            fseek(fp_, kernel_offset_, SEEK_SET);
            LOG(ERROR) << "Error in writing kernel to function map item, "
                       << "written count: " << written_count << ", item count: " << 1;
            return false;
        }
        kernel_count_++;
    }
    function_offset_ += kernel_count_ * item_size0;

    for (const auto& project_items : function2binariesinfo_) {
        project_name = project_items.first;
        if (project_name.size() + 1 > PROJECT_LENGTH) {
            fseek(fp_, function_offset_, SEEK_SET);
            LOG(ERROR) << "Project name is too long, please reset "
                       << "PROJECT_LENGTH.";
            return false;
        }
        strcpy(function_storage.project_name, project_name.c_str());
        for (const auto& function_item : project_items.second) {
            function_name = function_item.first;
            if (function_name.size() + 1 > FUNCTION_LENGTH) {
                fseek(fp_, function_offset_, SEEK_SET);
                LOG(ERROR) << "Function name is too long, please reset "
                           << "FUNCTION_LENGTH.";
                return false;
            }
            strcpy(function_storage.function_name, function_name.c_str());
            function_storage.address_offset = function_item.second.address_offset;
            function_storage.binaries_size = function_item.second.binaries_size;
            written_count = fwrite(&function_storage, item_size1, 1, fp_);
            if (written_count != 1) {
                fseek(fp_, function_offset_, SEEK_SET);
                LOG(ERROR) << "Error in writing function to binaries map "
                           << "item, written count: " << written_count << ", item count: " << 1;
                return false;
            }
        }
    }

    int status = fseek(fp_, 0, SEEK_SET);
    if (status != 0) {
        LOG(ERROR) << "Error in resetting the file pointer to the beginning.";
        return false;
    }

    written_count = fwrite(&kernel_offset_, sizeof(uint32_t), 1, fp_);
    if (written_count != 1) {
        LOG(ERROR) << "Error in writing kernel to function map offset, "
                   << "written count: " << written_count << ", writing count: " << 1;
        return false;
    }
    written_count = fwrite(&kernel_count_, sizeof(uint32_t), 1, fp_);
    if (written_count != 1) {
        LOG(ERROR) << "Error in writing kernel count, written count: " << written_count << ", writing count: " << 1;
        return false;
    }

    written_count = fwrite(&function_offset_, sizeof(uint32_t), 1, fp_);
    if (written_count != 1) {
        LOG(ERROR) << "Error in writing function to binaries map offset, "
                   << "written count: " << written_count << ", writing count: " << 1;
        return false;
    }
    written_count = fwrite(&function_count_, sizeof(uint32_t), 1, fp_);
    if (written_count != 1) {
        LOG(ERROR) << "Error in writing function count, written count: " << written_count << ", writing count: " << 1;
        return false;
    }

    return true;
}

void KernelBinariesManager::setStatus(bool is_working) {
    is_working_ = is_working;
}

bool KernelBinariesManager::loadBinariesInfo() {
    int status = fseek(fp_, kernel_offset_, SEEK_SET);
    if (status != 0) {
        LOG(ERROR) << "Failed to locate kernel to function map info in kernel "
                   << "binaries file.";
        return false;
    }

    Kernel2FunctionItem kernel_storage;
    Function2BinariesItem function_storage;
    std::string project_name;
    std::string function_name;
    std::string kernel_name;
    KernelBinaryInfo binaries_info;
    size_t item_size0 = sizeof(Kernel2FunctionItem);
    size_t item_size1 = sizeof(Function2BinariesItem);

    size_t read_count;
    std::lock_guard<std::mutex> lock_guard(locker_);
    for (uint32_t i = 0; i < kernel_count_; i++) {
        read_count = fread(&kernel_storage, item_size0, 1, fp_);
        if (read_count != 1) {
            LOG(ERROR) << "Error in reading kernel to function map item " << i << ", read count: " << read_count
                       << ", item count: " << 1;
            return false;
        }
        kernel_name = kernel_storage.kernel_name;
        project_name = kernel_storage.project_name;
        function_name = kernel_storage.function_name;

        auto iter = kernel2function_.find(kernel_name);
        if (iter != kernel2function_.end()) {
            // LOG(INFO) << kernel_name << " has already been in the map.";
            continue;
        }
        kernel2function_[kernel_name] = make_pair(project_name, function_name);
    }

    uint32_t file_offset = ftell(fp_);
    if (file_offset != function_offset_) {
        LOG(ERROR) << " Inconsistency in " << BINARIES_FILE << ", the current location: " << file_offset
                   << ", desired location: " << function_offset_;
        return false;
    }

    for (uint32_t i = 0; i < function_count_; i++) {
        read_count = fread(&function_storage, item_size1, 1, fp_);
        if (read_count != 1) {
            LOG(ERROR) << "Error in reading function to binaries info map item " << i << ", read count: " << read_count
                       << ", item count: " << 1;
            return false;
        }
        project_name = function_storage.project_name;
        function_name = function_storage.function_name;
        binaries_info.address_offset = function_storage.address_offset;
        binaries_info.binaries_size = function_storage.binaries_size;

        auto& function2kernelinfo = function2binariesinfo_[project_name];
        auto iter = function2kernelinfo.find(function_name);
        if (iter != function2kernelinfo.end()) {
            // LOG(INFO) << function_name << " has already been in the map.";
            continue;
        }
        function2kernelinfo[function_name] = binaries_info;
    }

    return true;
}

bool KernelBinariesManager::retrieveKernel(const std::string& project_name, const std::string& kernel_name,
                                           size_t* binaries_length, unsigned char** binaries_data) {
    if (project_name.size() == 0) {
        LOG(ERROR) << "The project name is invalid.";
        return false;
    }

    if (kernel_name.size() == 0) {
        LOG(ERROR) << "The kernel name is invalid.";
        return false;
    }

    if (binaries_length == nullptr) {
        LOG(ERROR) << "The address of binaries length is invalid.";
        return false;
    }

    if (binaries_data == nullptr) {
        LOG(ERROR) << "The binaries buffer is invalid.";
        return false;
    }

    auto iter0 = kernel2function_.find(kernel_name);
    if (iter0 == kernel2function_.end()) {
        LOG(ERROR) << "The kernel " << kernel_name << " can't be found in the "
                   << "kernel binaries.";
        return false;
    }

    std::string project_name1 = kernel2function_[kernel_name].first;
    if (project_name != project_name1) {
        LOG(ERROR) << "The projects are mismatching, querying project: " << project_name
                   << ", found project: " << project_name1;
        return false;
    }
    std::string function_name = kernel2function_[kernel_name].second;

    auto iter1 = function2binariesinfo_.find(project_name);
    if (iter1 == function2binariesinfo_.end()) {
        LOG(ERROR) << "The project " << project_name << " can't be found in "
                   << "the kernel binaries.";
        return false;
    }

    auto& function2kernelinfo = function2binariesinfo_[project_name];
    auto iter2 = function2kernelinfo.find(function_name);
    if (iter2 == function2kernelinfo.end()) {
        LOG(ERROR) << "The function " << function_name << " can't be found in "
                   << "the kernel binaries.";
        return false;
    }

    function_offset_ = iter2->second.address_offset;
    *binaries_length = iter2->second.binaries_size;

    int status = fseek(fp_, function_offset_, SEEK_SET);
    if (status != 0) {
        LOG(ERROR) << "Failed to locate function binaries in kernel binaries.";
        return false;
    }

    (*binaries_data) = new unsigned char[*binaries_length];
    size_t read_bytes;
    read_bytes = fread((*binaries_data), 1, *binaries_length, fp_);
    if (read_bytes != *binaries_length) {
        LOG(ERROR) << "Error in reading kernel binaries, read bytes: " << read_bytes
                   << ", binaries bytes: " << *binaries_length;
        return false;
    }

    return true;
}

void KernelBinariesManager::releaseResource() {
    kernel2function_.clear();
    function2binariesinfo_.clear();

    if (fp_ == nullptr) {
        return;
    }

    int status = fclose(fp_);
    if (status == 0) {
        fp_ = nullptr;
    } else {
        LOG(ERROR) << "Error in close the kernel binaries file.";
    }
}

}}} // namespace ppl::common::ocl
