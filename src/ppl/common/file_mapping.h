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

#ifndef _ST_HPC_PPL_COMMON_FILE_MAPPING_H_
#define _ST_HPC_PPL_COMMON_FILE_MAPPING_H_

#include "ppl/common/retcode.h"

#ifdef _MSC_VER
#define NO_MINMAX
#define WIN32_LEAN_AND_MEAN
#define NOGDI
#include <windows.h>
#endif

namespace ppl { namespace common {

class FileMapping final {
private:
    static const uint32_t MAX_MSG_BUF_SIZE = 1024;

public:
    FileMapping();
    ~FileMapping();

    ppl::common::RetCode Init(const char* filename);
    const char* Data() const {
        return (const char*)buffer_;
    }
    uint64_t Size() const {
        return size_;
    }
    const char* GetErrorMessage() const {
        return error_message_;
    }

private:
#ifdef _MSC_VER
    HANDLE h_file_;
    HANDLE h_map_file_;
#else
    int file_fd_;
#endif
    void* buffer_;
    uint64_t size_;
    char error_message_[MAX_MSG_BUF_SIZE];
};

}} // namespace ppl::common

#endif
