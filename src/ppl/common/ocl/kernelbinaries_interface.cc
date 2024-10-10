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

#include "kernelbinaries_interface.h"
#include "kernelbinariesmanager.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#include "CL/cl.h"

#include "ppl/common/log.h"

namespace ppl { namespace common { namespace ocl {

static KernelBinariesManager binaries_manager;

bool detectKernelBinariesFile() {
#ifdef _WIN32
    DWORD fileAttributes = GetFileAttributesA(BINARIES_FILE);

    if (fileAttributes != INVALID_FILE_ATTRIBUTES && !(fileAttributes &
                                                       FILE_ATTRIBUTE_DIRECTORY)) {
        return true;
    }
#else
    if (access(BINARIES_FILE, R_OK) == 0) {
        return true;
    }
#endif

    return false;
}

bool initializeKernelBinariesManager(BinariesManagerStatus status) {
    if (status == BINARIES_COMPILE) {
        bool succeeded = binaries_manager.prepareManager(BINARIES_COMPILE);

        return succeeded;
    } else if (status == BINARIES_RETRIEVE) {
        bool status0 = detectKernelBinariesFile();
        if (!status0)
            return false;

        bool status1 = binaries_manager.prepareManager(BINARIES_RETRIEVE);
        if (!status1)
            return false;

        status1 = binaries_manager.loadBinariesInfo();
        if (!status1) {
            binaries_manager.setStatus(false);
            return false;
        }

        return true;
    } else {
        return false;
    }
}

bool buildKernelBinaries() {
    bool status = binaries_manager.isWorking();
    if (!status) {
        return false;
    }

    FrameChain* frame_chain = getSharedFrameChain();
    status = binaries_manager.buildFunctionFromSource(frame_chain);
    if (!status) {
        return false;
    }

    status = binaries_manager.storeFunctionBinaries(frame_chain);
    return status;
}

bool retrieveKernelBinaries(const std::string& project_name, const std::string& kernel_name, size_t* binaries_length,
                            unsigned char** binaries_data) {
    bool status = binaries_manager.isWorking();
    if (!status) {
        return false;
    }

    bool succeeded = binaries_manager.retrieveKernel(project_name, kernel_name, binaries_length, binaries_data);
    return succeeded;
}

void shutDownKernelBinariesManager(BinariesManagerStatus status) {
    bool succeeded = true;
    if (status == BINARIES_COMPILE) {
        if (binaries_manager.isWorking()) {
            succeeded = binaries_manager.storeMapstoFile();
        }
    }
    binaries_manager.releaseResource();
    binaries_manager.setStatus(false);

    if (!succeeded) {
        LOG(ERROR) << "Failed to shut down kernel binaries manager.";
    }
}

}}} // namespace ppl::common::ocl
