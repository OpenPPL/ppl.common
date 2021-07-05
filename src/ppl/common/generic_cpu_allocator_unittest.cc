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

#include "ppl/common/generic_cpu_allocator.h"
#include "gtest/gtest.h"
using namespace ppl::common;

TEST(GenericCpuAllocatorTest, alloc_and_free) {
    GenericCpuAllocator ar(64u);
    auto ret = ar.Alloc(100);
    EXPECT_NE(nullptr, ret);
    ar.Free(ret);
}

TEST(GenericCpuAllocatorTest, alignment) {
    uint32_t expected_alignment = 32;
    GenericCpuAllocator ar(expected_alignment);

    auto ret = ar.Alloc(189);
    EXPECT_NE(nullptr, ret);
    EXPECT_EQ(0, (uintptr_t)ret % expected_alignment);
    ar.Free(ret);
}
