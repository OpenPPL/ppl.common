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

#include "ppl/common/compact_addr_manager.h"
using namespace std;

namespace ppl { namespace common {

static void RemoveFromSize2Addr(uintptr_t addr, uint64_t size, map<uint64_t, set<uintptr_t>>* size2addr) {
    auto s2a_iter = size2addr->find(size);
    if (s2a_iter != size2addr->end()) {
        s2a_iter->second.erase(addr);
        if (s2a_iter->second.empty()) {
            size2addr->erase(s2a_iter);
        }
    }
}

static void AddFreeBlock(uintptr_t new_addr, uint64_t new_size, map<uint64_t, set<uintptr_t>>* size2addr,
                         map<uintptr_t, uint64_t>* addr2size) {
    auto ret_pair = size2addr->insert(make_pair(new_size, set<uintptr_t>()));
    ret_pair.first->second.insert(new_addr);
    addr2size->insert(make_pair(new_addr, new_size));
}

uintptr_t CompactAddrManager::AllocByAllocator(uint64_t needed) {
    auto alloc_res = ar_->Alloc(needed);
    if (alloc_res.first == UINTPTR_MAX) {
        return UINTPTR_MAX;
    }

    // merge with the max-addr block if possible
    auto max_addr_iter = addr2size_.rbegin();
    if ((max_addr_iter != addr2size_.rend()) &&
        (max_addr_iter->first + max_addr_iter->second == alloc_res.first)) {
        auto new_addr = max_addr_iter->first;
        auto new_size = max_addr_iter->second + alloc_res.second;

        RemoveFromSize2Addr(max_addr_iter->first, max_addr_iter->second, &size2addr_);
        addr2size_.erase((++max_addr_iter).base());
        AddFreeBlock(new_addr + needed, new_size - needed, &size2addr_, &addr2size_);

        return new_addr;
    }

    if (needed < alloc_res.second) {
        AddFreeBlock(alloc_res.first + needed, alloc_res.second - needed, &size2addr_, &addr2size_);
    }

    return alloc_res.first;
}

uintptr_t CompactAddrManager::AllocByVMAllocator(uint64_t needed) {
    auto end_addr = vmr_->GetReservedBase() + vmr_->GetAllocatedSize();
    auto ret_addr = end_addr;

    // finds the largest free address to see whether we can allocate from the end of allocated area
    auto max_addr_iter = addr2size_.rbegin();
    bool is_consecutive = (max_addr_iter != addr2size_.rend() &&
                           (max_addr_iter->first + max_addr_iter->second == end_addr));
    if (is_consecutive) {
        ret_addr = max_addr_iter->first;
        needed -= max_addr_iter->second;
    }

    uint64_t allocated = vmr_->Extend(needed);
    if (allocated == 0) {
        return UINTPTR_MAX;
    }

    // removes the previous largest addr block because it is merged with the newly allocated one
    if (is_consecutive) {
        RemoveFromSize2Addr(max_addr_iter->first, max_addr_iter->second, &size2addr_);
        addr2size_.erase((++max_addr_iter).base());
    }

    if (needed < allocated) {
        AddFreeBlock(end_addr + needed, allocated - needed, &size2addr_, &addr2size_);
    }

    return ret_addr;
}

uintptr_t CompactAddrManager::Alloc(uint64_t needed) {
    // find a best-fit bytes block in free blocks
    auto s2a_iter = size2addr_.lower_bound(needed);
    if (s2a_iter == size2addr_.end()) {
        if (ar_) {
            return AllocByAllocator(needed);
        }
        return AllocByVMAllocator(needed);
    }

    // if best-fit bytes block(s) are found, use the first one and remove it from free list
    auto addr_iter = s2a_iter->second.begin();
    auto res_addr = *addr_iter;
    s2a_iter->second.erase(addr_iter);
    if (s2a_iter->second.empty()) {
        size2addr_.erase(s2a_iter);
    }

    // find size of the chosen block and remove it from addr2size_
    auto a2s_iter = addr2size_.find(res_addr);
    auto block_rest_size = a2s_iter->second;
    addr2size_.erase(a2s_iter);

    // insert the rest of block into free list
    if (block_rest_size > needed) {
        AddFreeBlock(res_addr + needed, block_rest_size - needed, &size2addr_, &addr2size_);
    }

    return res_addr;
}

void CompactAddrManager::Free(uintptr_t addr, uint64_t size) {
    auto next_addr = addr + size;

    // find and merge with its successor
    auto a2s_iter = addr2size_.find(next_addr);
    if (a2s_iter != addr2size_.end()) {
        size += a2s_iter->second;
        RemoveFromSize2Addr(next_addr, a2s_iter->second, &size2addr_);
        addr2size_.erase(a2s_iter);
    }

    // find and merge with its predecessor
    a2s_iter = addr2size_.lower_bound(addr);
    if (a2s_iter == addr2size_.begin()) {
        a2s_iter = addr2size_.end();
    } else {
        --a2s_iter;
    }

    if (a2s_iter != addr2size_.end() && a2s_iter->first + a2s_iter->second == addr) {
        RemoveFromSize2Addr(a2s_iter->first, a2s_iter->second, &size2addr_);
        a2s_iter->second += size;

        auto ret_pair = size2addr_.insert(make_pair(a2s_iter->second, set<uintptr_t>()));
        ret_pair.first->second.insert(a2s_iter->first);
    } else {
        AddFreeBlock(addr, size, &size2addr_, &addr2size_);
    }
}

}} // namespace ppl::common
