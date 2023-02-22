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

#include <string.h>
#include <unistd.h>

#include "CL/cl.h"

#include "ppl/common/log.h"

namespace ppl { namespace common { namespace ocl {

KernelBinariesManager::KernelBinariesManager() : fp_(nullptr),
        binaries_offset_(0), function_number_(0) {
}

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

        // fseek(fp_, sizeof(size_t) * 2, SEEK_SET);
        binaries_offset_ += sizeof(size_t) * 2;
        function_number_  = 0;
        fwrite(&binaries_offset_, sizeof(size_t), 1, fp_);
        fwrite(&function_number_, sizeof(size_t), 1, fp_);

        return true;
    }
    else if (status == BINARIES_RETRIEVE) {
        fp_ = fopen(BINARIES_FILE, "rb");
        if (fp_ == nullptr) {
            LOG(ERROR) << "Failed to open the file: " << BINARIES_FILE;
            return false;
        }

        size_t read_size;
        read_size = fread(&binaries_offset_, sizeof(size_t), 1, fp_);
        if (read_size != sizeof(size_t)) {
            LOG(ERROR) << "Error in reading binaries offset, returned size: "
                       << read_size << ", item size: " << sizeof(size_t);
            return false;
        }

        read_size = fread(&function_number_, sizeof(size_t), 1, fp_);
        if (read_size != sizeof(size_t)) {
            LOG(ERROR) << "Error in reading binaries number, returned size: "
                       << read_size << ", item size: " << sizeof(size_t);
            return false;
        }

        return true;
    }
    else {
        return false;
    }
}

bool KernelBinariesManager::buildFunctionFromSource(FrameChain* frame_chain,
                                                    const char* source_str) {
    if (frame_chain == nullptr) {
        LOG(ERROR) << "The opencl frame chain is invalid.";
        return false;
    }

    size_t kernel_length = strlen(source_str);
    if (kernel_length == 0) {
        LOG(ERROR) << "The function name is invalid.";
        return false;
    }

    cl_program program;
    cl_int error_code;
    cl_context context = frame_chain->getContext();
    program = clCreateProgramWithSource(context, 1, &source_str, &kernel_length,
                                        &error_code);
    if (error_code != CL_SUCCESS) {
        LOG(ERROR) << "Call clCreateProgramWithSource() failed with code: "
                   << error_code;
        return false;
    }
    frame_chain->setProgram(program);
    clRetainProgram(program);  // ???

    cl_device_id device_id = frame_chain->getDeviceId();
    std::string build_options = frame_chain->getCompileOptions();
    error_code = clBuildProgram(program, 1, &device_id, build_options.c_str(),
                                nullptr, nullptr);
    if (error_code != CL_SUCCESS) {
        size_t log_length;
        cl_int code = clGetProgramBuildInfo(program, device_id,
                          CL_PROGRAM_BUILD_LOG, 0, nullptr, &log_length);
        if (code != CL_SUCCESS) {
            LOG(ERROR) << "Call clGetProgramBuildInfo() failed with code: "
                       << code;
            return false;
        }

        std::string log_buffer((int)log_length, '0');
        code = clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG,
                                     log_length, (char*)log_buffer.data(),
                                     nullptr);
        if (code != CL_SUCCESS) {
            LOG(ERROR) << "Call clGetProgramBuildInfo() failed with code: "
                       << code;
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
    auto &function2kernelinfo = function2kernelbinaries_[project_name];
    auto iter = function2kernelinfo.find(function_name);
    if (iter != function2kernelinfo.end()) {
        LOG(INFO) << function_name << " has already been build and stored.";
        return true;
    }

    cl_program program = frame_chain->getProgram();
    size_t binary_size;
    size_t returned_size;
    error_code = clGetProgramInfo(program, CL_PROGRAM_BINARY_SIZES,
                                  sizeof(size_t), &binary_size, &returned_size);
    if (error_code != CL_SUCCESS) {
        LOG(ERROR) << "Call clGetProgramInfo() failed with code: "
                   << error_code;
        return false;
    }
    // std::cout << "binary sizes: " << binary_size << std::endl;
    // std::cout << "returned size: " << returned_size << std::endl;

    unsigned char** program_binaries = new unsigned char*[1];
    program_binaries[0] = new unsigned char[binary_size];
    error_code = clGetProgramInfo(program, CL_PROGRAM_BINARIES,
                                  sizeof(unsigned char**), program_binaries,
                                  &returned_size);
    if (error_code != CL_SUCCESS) {
        LOG(ERROR) << "Call clGetProgramInfo() failed with code: "
                   << error_code;
        return false;
    }
    if (returned_size != binary_size) {
        LOG(ERROR) << "Extracting kernel, returned size: " << returned_size
                   << ", binary size: " << binary_size;
        return false;
        // std::cout << "The binary code is returned." << std::endl;
        // std::cout << "returned size: " << returned_size << std::endl;
    }

    returned_size = fwrite(program_binaries[0], 1, binary_size, fp_);
    if (returned_size != binary_size) {
        fseek(fp_, 0 - returned_size, SEEK_CUR);
        LOG(ERROR) << "Error in writing file, returned size: " << returned_size
                   << ", binary size: " << binary_size;
        return false;
        // std::cout << "The binary code is returned." << std::endl;
        // std::cout << "returned size: " << returned_size << std::endl;
    }

    KernelBinaryInfo binaries_info;
    binaries_info.address_offset = binaries_offset_;
    binaries_info.size = binary_size;
    function2kernelinfo[function_name] = binaries_info;

    binaries_offset_ += binary_size;
    function_number_++;

    delete [] (program_binaries[0]);
    delete [] program_binaries;
    clReleaseProgram(program);

    return true;
}

bool KernelBinariesManager::storeMaptoFile() {
    KernelBinaryItem binary_item;
    std::string project_name;
    std::string function_name;
    size_t written_size;
    size_t item_size = sizeof(KernelBinaryItem);

    std::lock_guard<std::mutex> lock_guard(locker_);
    for (const auto &project_items : function2kernelbinaries_) {
        project_name = project_items.first;
        if (project_name.size() + 1 < LENGTH) {
            fseek(fp_, binaries_offset_, SEEK_SET);
            LOG(ERROR) << "Project name is too long, please reset LENGTH.";
            return false;
        }
        strcpy(binary_item.project_name, project_name.c_str());
        for (const auto &function_item : project_items.second) {
            function_name = function_item.first;
            if (function_name.size() + 1 < LENGTH) {
                fseek(fp_, binaries_offset_, SEEK_SET);
                LOG(ERROR) << "Function name is too long, please reset LENGTH.";
                return false;
            }
            strcpy(binary_item.function_name, function_name.c_str());
            binary_item.address_offset = function_item.second.address_offset;
            binary_item.size = function_item.second.size;
            written_size = fwrite(&binary_item, item_size, 1, fp_);
            if (written_size != item_size) {
                fseek(fp_, binaries_offset_, SEEK_SET);
                LOG(ERROR) << "Error in writing map item, returned size: "
                           << written_size << ", item size: " << item_size;
                return false;
                // std::cout << "The binary code is returned." << std::endl;
                // std::cout << "returned size: " << written_size << std::endl;
            }
        }
    }

    fseek(fp_, 0, SEEK_SET);
    fwrite(&binaries_offset_, sizeof(size_t), 1, fp_);
    fwrite(&function_number_, sizeof(size_t), 1, fp_);
    fclose(fp_);

    return true;
}

bool KernelBinariesManager::loadMapfromFile() {
    int status = fseek(fp_, binaries_offset_, SEEK_SET);
    if (status != 0) {
        LOG(ERROR) << "Failed to locate map info in kernel binaries file.";
        return false;
    }

    KernelBinaryItem binary_item;
    std::string project_name;
    std::string function_name;
    KernelBinaryInfo binaries_info;
    size_t item_size = sizeof(KernelBinaryItem);

    size_t read_size;
    std::lock_guard<std::mutex> lock_guard(locker_);
    for (size_t i = 0; i < function_number_; i++) {
        read_size = fread(&binary_item, item_size, 1, fp_);
        if (read_size != item_size) {
            LOG(ERROR) << "Error in reading map item " << i
                       << ", returned size: " << read_size << ", item size: "
                       << item_size;
            return false;
        }
        project_name  = binary_item.project_name;
        function_name = binary_item.function_name;
        binaries_info.address_offset = binary_item.address_offset;
        binaries_info.size = binary_item.size;

        auto &function2kernelinfo = function2kernelbinaries_[project_name];
        auto iter = function2kernelinfo.find(function_name);
        if (iter != function2kernelinfo.end()) {
            LOG(INFO) << function_name << " has already been in the map.";
            continue;
        }
        function2kernelinfo[function_name] = binaries_info;
    }

    return true;
}

bool KernelBinariesManager::retrieveKernel(const std::string &project_name,
        const std::string &kernel_name, size_t* binaries_length,
        unsigned char* binaries_data) {
    if (project_name.size() == 0) {
        LOG(ERROR) << "The project name is invalid.";
        return false;
    }

    if (kernel_name.size() == 0) {
        LOG(ERROR) << "The kernel name is invalid.";
        return false;
    }

    if (binaries_length == nullptr) {
        LOG(ERROR) << "The binaries length is invalid.";
        return false;
    }

    if (binaries_data == nullptr) {
        LOG(ERROR) << "The binaries buffer is invalid.";
        return false;
    }

    auto iter0 = function2kernelbinaries_.find(project_name);
    if (iter0 == function2kernelbinaries_.end()) {
        return false;
    }

    auto &function2kernelinfo = function2kernelbinaries_[project_name];
    auto iter1 = function2kernelinfo.cbegin();
    while (iter1 != function2kernelinfo.cend()) {
        std::string function_name = iter1->first;
        if (kernel_name.find(function_name) != std::string::npos) {
            binaries_offset_ = iter1->second.address_offset;
            *binaries_length = iter1->second.size;

            int status = fseek(fp_, binaries_offset_, SEEK_SET);
            if (status != 0) {
                LOG(ERROR) << "Failed to locate map info in kernel binaries file.";
                return false;
            }

            size_t read_size;
            read_size = fread(binaries_data, 1, binaries_offset_, fp_);
            if (read_size != binaries_offset_) {
                LOG(ERROR) << "Error in reading kernel binaries, returned size: "
                        << read_size << ", item size: " << binaries_offset_;
                return false;
            }

            return true;
        }
    }

    return false;
}

void KernelBinariesManager::destroyMap() {
    function2kernelbinaries_.clear();

    fclose(fp_);
}

static KernelBinariesManager binaries_manager;

bool initializeKernelBinariesManager(BinariesManagerStatus status) {
    bool succeeded = binaries_manager.prepareManager(status);

    return succeeded;
}

bool processAFunction(FrameChain* frame_chain, const char* source_str) {
    bool succeeded = binaries_manager.buildFunctionFromSource(frame_chain,
                                                              source_str);
    if (succeeded == false) {
        return false;
    }

    succeeded = binaries_manager.storeFunctionBinaries(frame_chain);

    return succeeded;
}

bool detectKernelBinariesFile() {
    if (access(BINARIES_FILE, R_OK) == 0) {
        return true;
    }

    return false;
}

bool restoreKernelBianriesMap() {
    bool succeeded = binaries_manager.loadMapfromFile();

    return succeeded;
}

bool retrieveKernel(const std::string &project_name,
                    const std::string &kernel_name, size_t* binaries_length,
                    unsigned char* binaries_data) {
    bool succeeded = binaries_manager.retrieveKernel(project_name, kernel_name,
                        binaries_length, binaries_data);

    return succeeded;
}

bool shutDownKernelBinariesManager(BinariesManagerStatus status) {
    if (status == BINARIES_COMPILE) {
        bool succeeded = binaries_manager.storeMaptoFile();

        return succeeded;
    }
    else if (status == BINARIES_RETRIEVE) {
        binaries_manager.destroyMap();

        return true;
    }
    else {
        return false;
    }
}

}}}
