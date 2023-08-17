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

#include "ppl/common/mmap.h"

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
#include <cstdio> // for sprintf
#include <utility> // std::move

namespace ppl { namespace common {

Mmap::Mmap()
#ifdef _MSC_VER
    : h_file_(nullptr)
    , h_map_file_(nullptr)
#else
    : fd_(-1)
#endif
    , base_(nullptr)
    , start_(nullptr)
    , size_(0) {
}

void Mmap::Destroy() {
    if (base_) {
#ifdef _MSC_VER
        if (h_map_file_) {
            UnmapViewOfFile(base_);
            CloseHandle(h_map_file_);
            CloseHandle(h_file_);
        } else {
            free(base_);
        }
#else
        if (fd_ >= 0) {
            munmap(base_, size_);
            close(fd_);
        } else {
            free(base_);
        }
#endif
    }
}

Mmap::~Mmap() {
    Destroy();
}

void Mmap::DoMove(Mmap&& fm) {
#ifdef _MSC_VER
    h_file_ = fm.h_file_;
    h_map_file_ = fm.h_map_file_;
    fm.h_file_ = nullptr;
    fm.h_map_file_ = nullptr;
#else
    fd_ = fm.fd_;
    fm.fd_ = -1;
#endif

    base_ = fm.base_;
    start_ = fm.start_;
    size_ = fm.size_;

    fm.base_ = nullptr;
    fm.start_ = nullptr;
    fm.size_ = 0;
}

Mmap::Mmap(Mmap&& fm) {
    DoMove(std::move(fm));
}

void Mmap::operator=(Mmap&& fm) {
    Destroy();
    DoMove(std::move(fm));
}

RetCode Mmap::Init(uint64_t size) {
    base_ = malloc(size);
    if (!base_) {
        return RC_OUT_OF_MEMORY;
    }
    start_ = base_;
    size_ = size;
    return RC_SUCCESS;
}

#ifdef _MSC_VER
RetCode Mmap::Init(const char* filename, uint32_t permission, uint64_t offset, uint64_t length) {
    DWORD flags = 0;
    if (permission & Mmap::READ) {
        flags |= GENERIC_READ;
    }
    if (permission & Mmap::WRITE) {
        flags |= GENERIC_WRITE;
    }
    h_file_ = CreateFile(filename, flags, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
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

    if (offset >= file_size) {
        sprintf(error_message_, "offset[%llu] >= file size[%lu]\n", offset, file_size);
        return RC_INVALID_VALUE;
    }

    auto max_length = file_size - offset;
    if (length > max_length) {
        length = max_length;
    }

    flags = 0;
    if (permission & Mmap::WRITE) {
        flags = PAGE_READWRITE;
    } else if (permission & Mmap::READ) {
        flags = PAGE_READONLY;
    }
    /*
      https://docs.microsoft.com/en-us/windows/win32/memory/creating-a-file-mapping-object

      Since it does not cost you any system resources to create a larger file mapping object, create a file mapping
      object that is the size of the file (set the dwMaximumSizeHigh and dwMaximumSizeLow parameters of
      CreateFileMapping both to zero) even if you do not expect to view the entire file. The cost in system resources
      comes in creating the views and accessing them.
    */
    h_map_file_ = CreateFileMapping(h_file_, nullptr, flags, 0, 0, nullptr);
    if (!h_map_file_) {
        FormatMessage(FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_SYSTEM, nullptr, GetLastError(), 0,
                      error_message_, MAX_MSG_BUF_SIZE, nullptr);
        goto errout;
    }

    SYSTEM_INFO sys_info; // system information; used to get granularity
    GetSystemInfo(&sys_info);

    uint64_t mapping_start_offset = (offset / sys_info.dwAllocationGranularity) * sys_info.dwAllocationGranularity;
    DWORD file_offset_high = (mapping_start_offset >> 32), file_offset_low = (mapping_start_offset & 0xffffffff);

    flags = 0;
    if (permission & Mmap::READ) {
        flags |= FILE_MAP_READ;
    }
    if (permission & Mmap::WRITE) {
        flags |= FILE_MAP_WRITE;
    }
    base_ = MapViewOfFile(h_map_file_, flags, file_offset_high, file_offset_low, length);
    if (!base_) {
        FormatMessage(FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_SYSTEM, nullptr, GetLastError(), 0,
                      error_message_, MAX_MSG_BUF_SIZE, nullptr);
        goto errout2;
    }

    start_ = (char*)base_ + (offset - mapping_start_offset);
    size_ = length;
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
RetCode Mmap::Init(const char* filename, uint32_t permission, uint64_t offset, uint64_t length) {
    auto page_size = sysconf(_SC_PAGE_SIZE);
    auto mapping_start_offset = (offset / page_size) * page_size;
    struct stat file_stat_info;

    int flags = O_CLOEXEC;
    if ((permission & Mmap::READ) && (permission & Mmap::WRITE)) {
        flags |= O_RDWR;
    } else if (permission & Mmap::WRITE) {
        flags |= O_WRONLY;
    } else if (permission & Mmap::READ) {
        flags |= O_RDONLY;
    }
    int fd = open(filename, flags);
    if (fd < 0) {
        goto errout1;
    }

    memset(&file_stat_info, 0, sizeof(file_stat_info));
    if (fstat(fd, &file_stat_info) < 0 || file_stat_info.st_size < 0) {
        goto errout2;
    }

    {
        uint64_t file_size = file_stat_info.st_size;
        if (offset >= file_size) {
            sprintf(error_message_, "offset[%lu] >= file size[%lu]\n", offset, file_size);
            return RC_INVALID_VALUE;
        }

        auto max_length = file_size - offset;
        if (length > max_length) {
            length = max_length;
        }
    }

    flags = 0;
    if (permission & Mmap::READ) {
        flags |= PROT_READ;
    }
    if (permission & Mmap::WRITE) {
        flags |= PROT_WRITE;
    }
    base_ = mmap(NULL, length, flags, MAP_PRIVATE, fd, mapping_start_offset);
    if (base_ != MAP_FAILED) {
        fd_ = fd;
        start_ = (char*)base_ + (offset - mapping_start_offset);
        size_ = length;
        return RC_SUCCESS;
    }

errout2:
    close(fd);
errout1:
    strcpy(error_message_, strerror(errno));
    return RC_INVALID_VALUE;
}
#endif

}} // namespace ppl::common
