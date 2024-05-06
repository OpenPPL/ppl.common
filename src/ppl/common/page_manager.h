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

#ifndef _ST_HPC_PPL_COMMON_PAGE_MANAGER_H_
#define _ST_HPC_PPL_COMMON_PAGE_MANAGER_H_

#include "ppl/common/retcode.h"

#include <cstdint>
#include <map>
#include <set>
#include <vector>
#include <memory>

namespace ppl { namespace common {

class PageManager final {
public:
    PageManager() {}
    void Init(uint64_t max_size);
    RetCode Alloc(uint64_t needed, std::vector<uint64_t>* page_list);
    void Free(const uint64_t* page_list, uint32_t page_size);
    uint64_t GetAvail() {
        return max_ - used_;
    }

private:
    uint64_t max_ = 0;
    uint64_t used_ = 0;
    std::map<uint64_t, uint64_t> addr2size_;
    std::map<uint64_t, std::set<uint64_t>, std::greater<uint64_t>> size2addr_;
};

}}

#endif