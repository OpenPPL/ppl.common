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

#include "ppl/common/file_mapping.h"

#ifndef _MSC_VER
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#endif //! non windows
#include <cstring>
#include <cstdlib>

namespace ppl { namespace common {

FileMapping::FileMapping()
#ifdef _MSC_VER
    : h_file_(nullptr)
    , h_map_file_(nullptr)
#else
    : file_fd_(0)
#endif
    , buffer_(nullptr)
    , size_(0) {
}

FileMapping::~FileMapping() {
    if (buffer_) {
#ifdef _MSC_VER
        UnmapViewOfFile(buffer_);
        CloseHandle(h_map_file_);
        CloseHandle(h_file_);
#else
        munmap(buffer_, size_);
        close(file_fd_);
#endif
    }
}

#ifdef _MSC_VER
RetCode FileMapping::Init(const char* filename) {
    h_file_ =
        CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (h_file_ == INVALID_HANDLE_VALUE) {
        FormatMessage(FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_SYSTEM, nullptr, GetLastError(), 0,
                      error_message_, MAX_MSG_BUF_SIZE, nullptr);
        return RC_INVALID_VALUE;
    }

    DWORD file_size = GetFileSize(h_file_, nullptr);
    if (file_size == 0) {
        strcpy(error_message_, "model file size is 0.");
        goto errout;
    }

    h_map_file_ = CreateFileMapping(h_file_, nullptr, PAGE_READONLY, 0, 0, nullptr);
    if (!h_map_file_) {
        FormatMessage(FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_SYSTEM, nullptr, GetLastError(), 0,
                      error_message_, MAX_MSG_BUF_SIZE, nullptr);
        goto errout;
    }

    buffer_ = MapViewOfFile(h_map_file_, FILE_MAP_READ, 0, 0, 0);
    if (!buffer_) {
        FormatMessage(FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_SYSTEM, nullptr, GetLastError(), 0,
                      error_message_, MAX_MSG_BUF_SIZE, nullptr);
        goto errout2;
    }

    size_ = file_size;
    return RC_SUCCESS;

errout2:
    CloseHandle(h_map_file_);
    h_map_file_ = nullptr;
errout:
    CloseHandle(h_file_);
    h_file_ = nullptr;
    return RC_INVALID_VALUE;
}
#else
RetCode FileMapping::Init(const char* filename) {
    void* buf = nullptr;
    struct stat file_stat_info;
    int fd = open(filename, O_RDONLY | O_CLOEXEC);
    if (fd < 0) {
        goto Error_Handler1;
    }
    memset(&file_stat_info, 0, sizeof(file_stat_info));
    if (fstat(fd, &file_stat_info) < 0 || file_stat_info.st_size < 0) {
        goto Error_Handler2;
    }
    buf = mmap(NULL, file_stat_info.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (buf) {
        this->file_fd_ = fd;
        this->buffer_ = buf;
        this->size_ = file_stat_info.st_size;
        return RC_SUCCESS;
    }

Error_Handler2:
    close(fd);
Error_Handler1:
    strcpy(error_message_, strerror(errno));
    return RC_INVALID_VALUE;
}
#endif

}} // namespace ppl::common
