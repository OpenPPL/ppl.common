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

#include "ppl/common/compact_memory_manager.h"
using namespace std;

namespace ppl { namespace common {

CompactMemoryManager::CompactMemoryManager(Allocator* ar, uint64_t block_bytes)
    : block_bytes_(block_bytes), allocator_(ar) {
    blocks_.reserve(64);
}

CompactMemoryManager::~CompactMemoryManager() {
    if (allocator_) {
        for (auto x = blocks_.begin(); x != blocks_.end(); ++x) {
            allocator_->Free(*x);
        }
    }
}

void CompactMemoryManager::Clear() {
    if (allocator_) {
        for (auto x = blocks_.begin(); x != blocks_.end(); ++x) {
            allocator_->Free(*x);
        }
    }

    buffered_bytes_ = 0;
    allocated_bytes_ = 0;
    addr2bytes_.clear();
    bytes2addr_.clear();
    blocks_.clear();
}

static void RemoveFromBytes2Addr(void* addr, uint64_t bytes, map<uint64_t, set<void*>>* bytes2addr) {
    auto b2a_iter = bytes2addr->find(bytes);
    if (b2a_iter != bytes2addr->end()) {
        b2a_iter->second.erase(addr);
        if (b2a_iter->second.empty()) {
            bytes2addr->erase(b2a_iter);
        }
    }
}

static void AddFreeBlock(void* new_addr, uint64_t new_bytes, map<uint64_t, set<void*>>* bytes2addr,
                         map<void*, uint64_t>* addr2bytes) {
    auto ret_pair = bytes2addr->insert(make_pair(new_bytes, set<void*>()));
    ret_pair.first->second.insert(new_addr);
    addr2bytes->insert(make_pair(new_addr, new_bytes));
}

static inline uint64_t Align(uint64_t x, uint64_t n) {
    return (x + n - 1) & (~(n - 1));
}

void* CompactMemoryManager::AllocByAllocator(uint64_t bytes_needed) {
    // create a new block if not found
    uint64_t bytes_allocated = Align(bytes_needed, block_bytes_);
    auto new_block = allocator_->Alloc(bytes_allocated);
    if (!new_block) {
        return nullptr;
    }
    buffered_bytes_ += bytes_allocated;
    blocks_.push_back(new_block);

    // merge with the max-addr block if possible
    auto max_addr_iter = addr2bytes_.rbegin();
    if ((max_addr_iter != addr2bytes_.rend()) &&
        ((char*)max_addr_iter->first + max_addr_iter->second == new_block)) {
        auto new_addr = max_addr_iter->first;
        auto new_size = max_addr_iter->second + bytes_allocated;

        RemoveFromBytes2Addr(max_addr_iter->first, max_addr_iter->second, &bytes2addr_);
        addr2bytes_.erase((++max_addr_iter).base());

        AddFreeBlock((char*)new_addr + bytes_needed, new_size - bytes_needed, &bytes2addr_, &addr2bytes_);
        return new_addr;
    }

    if (bytes_needed < bytes_allocated) {
        AddFreeBlock((char*)new_block + bytes_needed, bytes_allocated - bytes_needed, &bytes2addr_, &addr2bytes_);
    }
    return new_block;
}

void* CompactMemoryManager::AllocByVMAllocator(uint64_t bytes_needed) {
    auto end_addr = (char*)vmr_->GetReservedBaseAddr() + vmr_->GetUsedBytes();
    void* ret_addr = end_addr;

    // finds the largest free address to see whether we can allocate from the end of allocated area
    auto max_addr_iter = addr2bytes_.rbegin();
    bool is_consecutive = (max_addr_iter != addr2bytes_.rend() &&
                           ((char*)max_addr_iter->first + max_addr_iter->second == end_addr));
    if (is_consecutive) {
        ret_addr = max_addr_iter->first;
        bytes_needed -= max_addr_iter->second;
    }

    uint64_t bytes_allocated = vmr_->Extend(bytes_needed);
    if (bytes_allocated == 0) {
        return nullptr;
    }
    buffered_bytes_ += bytes_allocated;

    // removes the previous largest addr block because it is merged with the newly allocated block
    if (is_consecutive) {
        RemoveFromBytes2Addr(max_addr_iter->first, max_addr_iter->second, &bytes2addr_);
        addr2bytes_.erase((++max_addr_iter).base());
    }

    if (bytes_needed < bytes_allocated) {
        AddFreeBlock(end_addr + bytes_needed, bytes_allocated - bytes_needed, &bytes2addr_, &addr2bytes_);
    }

    return ret_addr;
}

void* CompactMemoryManager::Alloc(uint64_t bytes_needed) {
    // find a best-fit bytes block in free blocks
    auto b2a_iter = bytes2addr_.lower_bound(bytes_needed);
    if (b2a_iter == bytes2addr_.end()) {
        void* ret;
        if (allocator_) {
            ret = AllocByAllocator(bytes_needed);
        } else {
            ret = AllocByVMAllocator(bytes_needed);
        }
        if (ret) {
            allocated_bytes_ += bytes_needed;
        }
        return ret;
    }

    // if best-fit bytes block(s) are found, use the first one and remove it from free list
    auto addr_iter = b2a_iter->second.begin();
    auto res_addr = *addr_iter;
    b2a_iter->second.erase(addr_iter);
    if (b2a_iter->second.empty()) {
        bytes2addr_.erase(b2a_iter);
    }

    // find size of the chosen block and remove it from addr2bytes_
    auto a2b_iter = addr2bytes_.find(res_addr);

    // insert the rest of block into free list
    if (a2b_iter->second > bytes_needed) {
        AddFreeBlock((char*)res_addr + bytes_needed, a2b_iter->second - bytes_needed, &bytes2addr_, &addr2bytes_);
    }

    addr2bytes_.erase(a2b_iter);

    allocated_bytes_ += bytes_needed;
    return res_addr;
}

void CompactMemoryManager::Free(void* addr, uint64_t bytes) {
    void* next_addr = (char*)addr + bytes;

    // find and merge with its successor
    auto a2b_iter = addr2bytes_.find(next_addr);
    if (a2b_iter != addr2bytes_.end()) {
        bytes += a2b_iter->second;
        RemoveFromBytes2Addr(next_addr, a2b_iter->second, &bytes2addr_);
        addr2bytes_.erase(a2b_iter);
    }

    // find and merge with its predecessor
    a2b_iter = addr2bytes_.lower_bound(addr);
    if (a2b_iter == addr2bytes_.begin()) {
        a2b_iter = addr2bytes_.end();
    } else {
        --a2b_iter;
    }

    if (a2b_iter != addr2bytes_.end() && (char*)a2b_iter->first + a2b_iter->second == addr) {
        RemoveFromBytes2Addr(a2b_iter->first, a2b_iter->second, &bytes2addr_);
        a2b_iter->second += bytes;

        auto ret_pair = bytes2addr_.insert(make_pair(a2b_iter->second, set<void*>()));
        ret_pair.first->second.insert(a2b_iter->first);
    } else {
        AddFreeBlock(addr, bytes, &bytes2addr_, &addr2bytes_);
    }

    allocated_bytes_ -= bytes;
}

}} // namespace ppl::common
