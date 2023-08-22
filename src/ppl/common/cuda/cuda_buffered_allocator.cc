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

#include "cuda_buffered_allocator.h"
#include "ppl/common/log.h"
using namespace std;

namespace ppl { namespace common {

static inline uint64_t Align(uint64_t x, uint64_t n) {
    return (x + n - 1) & (~(n - 1));
}

void CudaBufferedAllocator::Destroy() {
    if (addr_ == 0) {
        return;
    }

    CUresult rc;
    const char* errmsg = nullptr;

    if (!handle_list_.empty()) {
        rc = cuMemUnmap(addr_, buffered_bytes_);
        if (rc != CUDA_SUCCESS) {
            cuGetErrorString(rc, &errmsg);
            LOG(ERROR) << "cuMemUnmap failed: " << errmsg;
        }

        for (auto x = handle_list_.begin(); x != handle_list_.end(); ++x) {
            rc = cuMemRelease(*x);
            if (rc != CUDA_SUCCESS) {
                cuGetErrorString(rc, &errmsg);
                LOG(ERROR) << "cuMemRelease failed: " << errmsg;
            }
        }
        handle_list_.clear();
    }

    rc = cuMemAddressFree(addr_, addr_len_);
    if (rc != CUDA_SUCCESS) {
        cuGetErrorString(rc, &errmsg);
        LOG(ERROR) << "cuMemAddressFree failed: " << errmsg;
    }

    addr_ = 0;
}

RetCode CudaBufferedAllocator::Init(int devid, uint64_t max_mem_bytes) {
    const char* errmsg = nullptr;

    auto rc = cuMemGetInfo(nullptr, &total_bytes_);
    if (rc != CUDA_SUCCESS) {
        cuGetErrorString(rc, &errmsg);
        LOG(ERROR) << "cuMemGetInfo failed: " << errmsg;
        return RC_OTHER_ERROR;
    }

    if (total_bytes_ > max_mem_bytes) {
        total_bytes_ = max_mem_bytes;
    }

    prop_.type = CU_MEM_ALLOCATION_TYPE_PINNED;
    prop_.location.type = CU_MEM_LOCATION_TYPE_DEVICE;
    prop_.location.id = devid;

    rc = cuMemGetAllocationGranularity(&granularity_, &prop_, CU_MEM_ALLOC_GRANULARITY_MINIMUM);
    if (rc != CUDA_SUCCESS) {
        cuGetErrorString(rc, &errmsg);
        LOG(ERROR) << "cuMemGetAllocationGranularity failed: " << errmsg;
        return RC_OTHER_ERROR;
    }

    access_desc_.location.type = CU_MEM_LOCATION_TYPE_DEVICE;
    access_desc_.location.id = devid;
    access_desc_.flags = CU_MEM_ACCESS_FLAGS_PROT_READWRITE;

    addr_len_ = Align(total_bytes_, granularity_);
    rc = cuMemAddressReserve(&addr_, addr_len_, 0, 0, 0);
    if (rc != CUDA_SUCCESS) {
        addr_ = 0;
        cuGetErrorString(rc, &errmsg);
        LOG(ERROR) << "cuMemAddressReserve failed: " << errmsg;
        return RC_OTHER_ERROR;
    }

    return RC_SUCCESS;
}

uint64_t CudaBufferedAllocator::Extend(uint64_t bytes_needed) {
    bytes_needed = Align(bytes_needed, granularity_);
    if (bytes_needed + buffered_bytes_ > total_bytes_) {
        LOG(ERROR) << "bytes_needed [" << bytes_needed << "] is larger than available ["
                   << total_bytes_ - buffered_bytes_ << "]";
        return 0;
    }

    const char* errmsg = nullptr;
    CUmemGenericAllocationHandle alloc_handle;
    auto rc = cuMemCreate(&alloc_handle, bytes_needed, &prop_, 0);
    if (rc != CUDA_SUCCESS) {
        cuGetErrorString(rc, &errmsg);
        LOG(ERROR) << "cuMemCreate [" << bytes_needed << "] bytes failed: " << errmsg;
        return 0;
    }

    auto start_addr = addr_ + buffered_bytes_;

    rc = cuMemMap(start_addr, bytes_needed, 0, alloc_handle, 0);
    if (rc != CUDA_SUCCESS) {
        cuGetErrorString(rc, &errmsg);
        LOG(ERROR) << "cuMemMap [" << bytes_needed << "] to addr [" << start_addr << "] failed: " << errmsg;
        return 0;
    }

    cuMemSetAccess(start_addr, bytes_needed, &access_desc_, 1);

    handle_list_.push_back(alloc_handle);
    buffered_bytes_ += bytes_needed;

    return bytes_needed;
}

}}
