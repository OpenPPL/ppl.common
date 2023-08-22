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

#ifndef _ST_HPC_PPL_COMMON_CUDA_BUFFERED_ALLOCATOR_H_
#define _ST_HPC_PPL_COMMON_CUDA_BUFFERED_ALLOCATOR_H_

#include "ppl/common/retcode.h"
#include "ppl/common/compact_addr_manager.h"
#include <cuda.h>
#include <limits>
#include <vector>

namespace ppl { namespace common {

class CudaBufferedAllocator final : public ppl::common::CompactAddrManager::VMAllocator {
public:
    CudaBufferedAllocator() {}
    ~CudaBufferedAllocator() {
        Destroy();
    }

    ppl::common::RetCode Init(int devid, uint64_t max_mem_bytes = UINT64_MAX);
    void Destroy();

    uintptr_t GetReservedBase() const override {
        return addr_;
    }
    uint64_t GetAllocatedSize() const override {
        return buffered_bytes_;
    }

    uint64_t Extend(uint64_t bytes_needed) override;

private:
    CUmemAllocationProp prop_ = {};
    CUmemAccessDesc access_desc_ = {};
    CUdeviceptr addr_ = 0;
    size_t addr_len_ = 0;
    size_t buffered_bytes_ = 0;
    size_t total_bytes_ = 0;
    uint64_t granularity_ = 0;
    std::vector<CUmemGenericAllocationHandle> handle_list_;

private:
    CudaBufferedAllocator(const CudaBufferedAllocator&) = delete;
    CudaBufferedAllocator& operator=(const CudaBufferedAllocator&) = delete;
};

}}

#endif
