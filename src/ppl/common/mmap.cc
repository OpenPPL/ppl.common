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
#include <cstdio> // sprintf
#include <utility> // std::move

#include "ppl/common/mmap.h"
#include "ppl/common/log.h"

namespace ppl { namespace common {

static constexpr uint32_t MAX_MSG_BUF_SIZE = 1024;

#ifdef _MSC_VER
static constexpr uint32_t INLINE_DATA_SIZE = sizeof(void*) + sizeof(HANDLE) + sizeof(HANDLE);
#else
static constexpr uint32_t INLINE_DATA_SIZE = sizeof(void*) + sizeof(int64_t);
#endif

Mmap::Mmap()
    : size_(0)
    , start_(nullptr)
    , base_(nullptr)
#ifdef _MSC_VER
    , h_file_(nullptr)
    , h_map_file_(nullptr)
#else
    , fd_(-1)
#endif
{}

void Mmap::Destroy() {
    if (start_ && start_ != &base_) {
#ifdef _MSC_VER
        if (h_map_file_) {
            UnmapViewOfFile(base_);
            CloseHandle(h_map_file_);
            CloseHandle(h_file_);
        }
#else
        if (fd_ >= 0) {
            munmap(base_, size_ + ((char*)start_ - (char*)base_));
            close(fd_);
        }
#endif
        else {
            free(start_);
        }
    }
}

Mmap::~Mmap() {
    Destroy();
}

void Mmap::DoMove(Mmap&& fm) {
    permission_ = fm.permission_;
    size_ = fm.size_;
    fm.size_ = 0;

    if (fm.start_ == &fm.base_) {
        start_ = &base_;
    } else {
        start_ = fm.start_;
    }
    fm.start_ = nullptr;

    base_ = fm.base_;
    fm.base_ = nullptr;

#ifdef _MSC_VER
    h_file_ = fm.h_file_;
    fm.h_file_ = nullptr;
    h_map_file_ = fm.h_map_file_;
    fm.h_map_file_ = nullptr;
#else
    fd_ = fm.fd_;
    fm.fd_ = -1;
#endif
}

Mmap::Mmap(Mmap&& fm) {
    if (&fm != this) {
        DoMove(std::move(fm));
    }
}

void Mmap::operator=(Mmap&& fm) {
    if (&fm != this) {
        Destroy();
        DoMove(std::move(fm));
    }
}

RetCode Mmap::Init(uint64_t size) {
    if (start_) {
        LOG(ERROR) << "duplicated init.";
        return RC_UNSUPPORTED;
    }

    if (size <= INLINE_DATA_SIZE) {
        start_ = &base_;
    } else {
        start_ = malloc(size);
        if (!start_) {
            return RC_OUT_OF_MEMORY;
        }
    }

    size_ = size;
    permission_ = READ | WRITE;
    return RC_SUCCESS;
}

#ifdef _MSC_VER
RetCode Mmap::Init(const char* filename, uint32_t permission, uint64_t offset, uint64_t length) {
    if (start_) {
        LOG(ERROR) << "duplicated init.";
        return RC_UNSUPPORTED;
    }

    DWORD flags = 0;
    if (permission & Mmap::READ) {
        flags |= GENERIC_READ;
    }
    if (permission & Mmap::WRITE) {
        flags |= GENERIC_WRITE;
    }
    h_file_ = CreateFile(filename, flags, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (h_file_ == INVALID_HANDLE_VALUE) {
        char message[MAX_MSG_BUF_SIZE];
        FormatMessage(FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_SYSTEM, nullptr, GetLastError(), 0,
                      message, MAX_MSG_BUF_SIZE, nullptr);
        LOG(ERROR) << "open file [" << filename << "] failed: " << message;
        return RC_INVALID_VALUE;
    }

    DWORD file_size = GetFileSize(h_file_, nullptr);
    if (file_size == 0) {
        LOG(ERROR) << "model file size is 0.";
        goto errout;
    }

    if (offset >= file_size) {
        LOG(ERROR) << "offset[" << offset << "] >= file size[" << file_size << "]";
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
        char message[MAX_MSG_BUF_SIZE];
        FormatMessage(FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_SYSTEM, nullptr, GetLastError(), 0,
                      message, MAX_MSG_BUF_SIZE, nullptr);
        LOG(ERROR) << "CreateFileMapping failed: " << message;
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
        char message[MAX_MSG_BUF_SIZE];
        FormatMessage(FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_SYSTEM, nullptr, GetLastError(), 0,
                      message, MAX_MSG_BUF_SIZE, nullptr);
        LOG(ERROR) << "MapViewOfFile failed: " << message;
        goto errout2;
    }

    start_ = (char*)base_ + (offset - mapping_start_offset);
    size_ = length;
    permission_ = permission;
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
    if (start_) {
        LOG(ERROR) << "duplicated init.";
        return RC_UNSUPPORTED;
    }

    const uint64_t page_size = sysconf(_SC_PAGE_SIZE);
    const uint64_t mapping_start_offset = (offset / page_size) * page_size;
    uint64_t mapping_length;

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
        LOG(ERROR) << "open file [" << filename << "] faled: " << strerror(errno);
        goto errout1;
    }

    {
        struct stat file_stat_info;
        memset(&file_stat_info, 0, sizeof(file_stat_info));
        if (fstat(fd, &file_stat_info) < 0) {
            LOG(ERROR) << "get stat of file [" << filename << "] failed: " << strerror(errno);
            goto errout2;
        }
        if (file_stat_info.st_size < 0) {
            LOG(ERROR) << "file [" << filename << "] size < 0";
            goto errout2;
        }

        uint64_t file_size = file_stat_info.st_size;
        if (offset >= file_size) {
            LOG(ERROR) << "offset[" << offset << "] >= file size[" << file_size << "].";
            goto errout2;
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

    mapping_length = length + (offset - mapping_start_offset);
    base_ = mmap(NULL, mapping_length, flags, MAP_PRIVATE, fd, mapping_start_offset);
    if (base_ != MAP_FAILED) {
        fd_ = fd;
        start_ = (char*)base_ + (offset - mapping_start_offset);
        size_ = length;
        permission_ = permission;
        return RC_SUCCESS;
    }

errout2:
    close(fd);
errout1:
    LOG(ERROR) << "mmap from [" << mapping_start_offset << "] with size [" << mapping_length << "] failed: "
               << strerror(errno);
    return RC_INVALID_VALUE;
}
#endif

}} // namespace ppl::common
