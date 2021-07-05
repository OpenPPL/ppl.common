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

#ifndef _ST_HPC_PPL_COMMON_COMPACT_MEMORY_MANAGER_H_
#define _ST_HPC_PPL_COMMON_COMPACT_MEMORY_MANAGER_H_

#include "ppl/common/allocator.h"
#include <vector>
#include <map>
#include <set>

namespace ppl { namespace common {

class CompactMemoryManager final {
public:
    /** @param block_bytes MUST be power of 2 */
    CompactMemoryManager(Allocator* ar, uint64_t block_bytes = 1048576);
    ~CompactMemoryManager();
    void* Alloc(uint64_t bytes);
    void Free(void* addr, uint64_t bytes);
    void Clear();
    uint64_t GetAllocatedBytes() const {
        return allocated_bytes_;
    }
    Allocator* GetAllocator() const {
        return allocator_;
    }

private:
    const uint64_t block_bytes_;
    uint64_t allocated_bytes_ = 0;
    Allocator* allocator_;
    std::map<void*, uint64_t> addr2bytes_;
    std::map<uint64_t, std::set<void*>> bytes2addr_;
    std::vector<void*> chunks_;
};

}} // namespace ppl::common

#endif
