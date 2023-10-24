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

#ifndef _ST_HPC_PPL_COMMON_MMAP_H_
#define _ST_HPC_PPL_COMMON_MMAP_H_

#include "ppl/common/retcode.h"
#include <limits> // UINT64_MAX

#ifdef _MSC_VER
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#define NOGDI
#include <cstddef>
#include <windows.h>
#endif

namespace ppl { namespace common {

class Mmap final {
public:
    static constexpr uint32_t READ = 1;
    static constexpr uint32_t WRITE = 2;

public:
    Mmap();
    ~Mmap();

    Mmap(Mmap&&);
    void operator=(Mmap&&);

    /**
       @param permission MUST be one of: READ_ONLY, WRITE_ONLY or READ_WRITE
       @param offset no alignment is required.
    */
    ppl::common::RetCode Init(const char* filename, uint32_t permission, uint64_t offset = 0,
                              uint64_t length = UINT64_MAX);
    /** @brief allocate a memory area of `size` */
    ppl::common::RetCode Init(uint64_t size);
    char* GetData() {
        return static_cast<char*>(start_);
    }
    const char* GetData() const {
        return static_cast<const char*>(start_);
    }
    uint64_t GetSize() const {
        return size_;
    }

    uint32_t GetPermission() const {
        return permission_;
    }

private:
    void DoMove(Mmap&& fm);
    void Destroy();

private:
    uint32_t permission_ = 0;
    uint64_t size_;
    void* start_;

    // ----- //

    /**
       in file-mapping mode the following fields serve as what they are.
       in mem mode they are used to store data whose size <= INLINE_DATA_SIZE.
    */

    void* base_;
#ifdef _MSC_VER
    HANDLE h_file_;
    HANDLE h_map_file_;
#else
    int64_t fd_;
#endif

    // ----- //

private:
    Mmap(const Mmap&) = delete;
    Mmap& operator=(const Mmap&) = delete;
};

}} // namespace ppl::common

#endif
