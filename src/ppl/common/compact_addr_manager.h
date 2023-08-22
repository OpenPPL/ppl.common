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

#ifndef _ST_HPC_PPL_COMMON_COMPACT_ADDR_MANAGER_H_
#define _ST_HPC_PPL_COMMON_COMPACT_ADDR_MANAGER_H_

#include <cstdint>
#include <map>
#include <set>

namespace ppl { namespace common {

class CompactAddrManager final {
public:
    class Allocator {
    public:
        virtual ~Allocator() {}
        virtual uint64_t GetAllocatedSize() const = 0;
        /** returns the starting addr and size allocated, or <UINTPTR_MAX, undefined> if failed. */
        virtual std::pair<uintptr_t, uint64_t> Alloc(uint64_t needed) = 0;
    };

    class VMAllocator {
    public:
        virtual ~VMAllocator() {}
        virtual uintptr_t GetReservedBase() const = 0;
        virtual uint64_t GetAllocatedSize() const = 0;
        /** acquires `needed` from the end position and returns the actual size allocated or 0 if failed. */
        virtual uint64_t Extend(uint64_t needed) = 0;
    };

public:
    CompactAddrManager(Allocator* ar) : ar_(ar) {}
    CompactAddrManager(VMAllocator* mgr) : vmr_(mgr) {}

    /** returns UINTPTR_MAX if failed. */
    uintptr_t Alloc(uint64_t size);
    void Free(uintptr_t addr, uint64_t size);

private:
    /** returns UINTPTR_MAX if failed. */
    uintptr_t AllocByAllocator(uint64_t needed);
    uintptr_t AllocByVMAllocator(uint64_t needed);

private:
    Allocator* ar_ = nullptr;
    VMAllocator* vmr_ = nullptr;
    std::map<uintptr_t, uint64_t> addr2size_;
    std::map<uint64_t, std::set<uintptr_t>> size2addr_;

private:
    CompactAddrManager(const CompactAddrManager&) = delete;
    CompactAddrManager& operator=(const CompactAddrManager&) = delete;
};

}} // namespace ppl::common

#endif
