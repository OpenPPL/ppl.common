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

#include "ppl/common/page_manager.h"

using namespace std;

namespace ppl { namespace common {

static void RemoveFreeBlock(int64_t addr, int64_t size,
                            map<int64_t, set<int64_t>, std::greater<int64_t>>* size2addr,
                            std::map<int64_t, int64_t>* addr2size) {
    addr2size->erase(addr);
    auto s2a_iter = size2addr->find(size);
    s2a_iter->second.erase(addr);
    if (s2a_iter->second.empty()) {
        size2addr->erase(s2a_iter);
    }
}

static void AddFreeBlock(int64_t new_addr, int64_t new_size,
                         map<int64_t, set<int64_t>, std::greater<int64_t>>* size2addr,
                         map<int64_t, int64_t>* addr2size) {
    auto ret_pair = size2addr->insert(make_pair(new_size, set<int64_t>()));
    ret_pair.first->second.insert(new_addr);
    addr2size->insert(make_pair(new_addr, new_size));
}

static void AddMergeFreeBlock(int64_t start_page, int64_t size,
                              map<int64_t, set<int64_t>, std::greater<int64_t>>* size2addr,
                              map<int64_t, int64_t>* addr2size) {
    // find and merge with its successor
    int64_t next_addr = start_page + size;
    auto a2s_iter = addr2size->find(next_addr);
    if (a2s_iter != addr2size->end()) {
        int64_t next_size = a2s_iter->second;
        RemoveFreeBlock(next_addr, next_size, size2addr, addr2size);
        size += next_size;
    }

    // find and merge with its predecessor
    a2s_iter = addr2size->lower_bound(start_page);
    if (a2s_iter == addr2size->begin()) {
        a2s_iter = addr2size->end();
    } else {
        --a2s_iter;
    }
    if (a2s_iter != addr2size->end()) {
        int64_t prev_addr = a2s_iter->first;
        int64_t prev_size = a2s_iter->second;
        if (prev_addr + prev_size == start_page) {
            RemoveFreeBlock(prev_addr, prev_size, size2addr, addr2size);
            start_page = prev_addr;
            size += prev_size;
        }
    }
    AddFreeBlock(start_page, size, size2addr, addr2size);
}

void PageManager::Init(int64_t max_size, int64_t page_size) {
    if (page_size <= 0 || max_size <= 0) {
        return;
    }
    max_ = max_size / page_size;
    page_size_ = page_size;
    AddFreeBlock(0, max_, &size2addr_, &addr2size_);
}

struct AddrInfo {
    AddrInfo(int64_t _addr, int64_t _size) : addr(_addr), size(_size) {}
    int64_t addr;
    int64_t size;
};

RetCode PageManager::Alloc(int64_t needed, std::vector<int64_t>* page_list) {
    if (needed == 0) {
        return RC_SUCCESS;
    }
    if (needed < 0) {
        return RC_INVALID_VALUE;
    }
    int64_t avail = max_ - used_;
    if (avail < needed) {
        return RC_OUT_OF_MEMORY;
    }

    used_ += needed;
    int64_t rest_need = needed;

    bool has_allocated = false;
    std::vector<AddrInfo> remove_list;
    std::vector<AddrInfo> add_list;
    for (auto s2a_iter = size2addr_.begin(); s2a_iter != size2addr_.end(); ++s2a_iter) {
        int64_t addr_size = s2a_iter->first;
        for (const auto addr : s2a_iter->second) {
            if (rest_need > 0) {
                if (addr_size <= rest_need) {
                    for (int64_t page_id = addr; page_id < addr + addr_size; ++page_id) {
                        page_list->push_back(page_id * page_size_);
                    }
                    remove_list.emplace_back(addr, addr_size);
                    rest_need -= addr_size;
                } else {
                    for (int64_t page_id = addr; page_id < addr + rest_need; ++page_id) {
                        page_list->push_back(page_id * page_size_);
                    }
                    remove_list.emplace_back(addr, addr_size);
                    int64_t new_size = addr_size - rest_need;
                    int64_t new_addr = addr + rest_need;
                    add_list.emplace_back(new_addr, new_size);
                    rest_need = 0;
                }
            } else {
                has_allocated = true;
                break;
            }
        }
        if (has_allocated) {
            break;
        }
    }

    for (const auto& remove_info : remove_list) {
        RemoveFreeBlock(remove_info.addr, remove_info.size, &size2addr_, &addr2size_);
    }
    for (const auto& add_info : add_list) {
        AddFreeBlock(add_info.addr, add_info.size, &size2addr_, &addr2size_);
    }

    return RC_SUCCESS;
}
void PageManager::Free(const int64_t* page_list, uint32_t page_size) {
    if (page_size <= 0) {
        return;
    }

    used_ -= page_size;
    int64_t prev_page = page_list[0], start_page = page_list[0], size = 1;
    for (size_t i = 1; i < page_size; ++i) {
        int64_t cur_page = page_list[i];
        if (cur_page - prev_page == 1) {
            size++;
            prev_page = cur_page;
        } else {
            AddMergeFreeBlock(start_page / page_size_, size, &size2addr_, &addr2size_);
            start_page = cur_page;
            prev_page = cur_page;
            size = 1;
        }
    }
    AddMergeFreeBlock(start_page / page_size_, size, &size2addr_, &addr2size_);
}

}} // namespace ppl::common