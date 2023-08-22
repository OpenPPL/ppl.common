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
#include "gtest/gtest.h"
using namespace std;
using namespace ppl::common;

class TestAllocator final : public CompactAddrManager::Allocator {
public:
    TestAllocator(uint64_t alignment) : alignment_(alignment) {}
    pair<uintptr_t, uint64_t> Alloc(uint64_t size) {
        auto ptr = aligned_alloc(size, alignment_);
        if (!ptr) {
            return make_pair(UINTPTR_MAX, 0);
        }
        allocated_size_ += size;
        return make_pair((uintptr_t)ptr, size);
    }
    uint64_t GetAllocatedSize() const override {
        return allocated_size_;
    }
private:
    uint64_t allocated_size_ = 0;
    const uint64_t alignment_;
};

TEST(CompactAddrManagerTest, alloc_and_free_1) {
    TestAllocator ar(64);
    CompactAddrManager mgr(&ar);

    uint32_t alloc_size = 100;
    auto ret = mgr.Alloc(alloc_size);
    EXPECT_NE(UINTPTR_MAX, ret);
    mgr.Free(ret, alloc_size);
}

TEST(CompactAddrManagerTest, reset) {
    TestAllocator ar(64);
    CompactAddrManager mgr(&ar);

    uint32_t alloc_size = 1048576;
    auto ret1 = mgr.Alloc(alloc_size);
    EXPECT_NE(UINTPTR_MAX, ret1);
    EXPECT_EQ(alloc_size, mgr.GetBufferedSize());

    alloc_size = alloc_size * 2;
    auto ret2 = mgr.Alloc(alloc_size);
    EXPECT_NE(UINTPTR_MAX, ret2);
    EXPECT_EQ(alloc_size * 3, mgr.GetBufferedSize());

    mgr.Free(ret1, alloc_size);
    mgr.Free(ret2, alloc_size);
}
