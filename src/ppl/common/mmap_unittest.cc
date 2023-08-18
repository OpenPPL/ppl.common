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

#include "gtest/gtest.h"
#include "ppl/common/mmap.h"
using namespace ppl::common;

TEST(MmapTest, init) {
    Mmap fm;
    EXPECT_EQ(RC_SUCCESS, fm.Init(__FILE__, Mmap::READ));
    EXPECT_NE(RC_SUCCESS, fm.Init("a", Mmap::READ));
}

TEST(MmapTest, data_and_size) {
    Mmap fm;
    EXPECT_EQ(RC_SUCCESS, fm.Init(__FILE__, Mmap::READ));
    EXPECT_TRUE(fm.GetSize() > 0);
    EXPECT_NE(nullptr, fm.GetData());
}

TEST(MmapTest, offset_and_length) {
    Mmap fm;
    const uint64_t offset = 22;
    const uint64_t length = 14;
    EXPECT_EQ(RC_SUCCESS, fm.Init(__FILE__, Mmap::READ, offset, length));
    EXPECT_EQ(14, fm.GetSize());
    EXPECT_NE(nullptr, fm.GetData());

    auto data = fm.GetData();
    EXPECT_EQ('c', data[0]);
    EXPECT_EQ('h', data[1]);
    EXPECT_EQ('e', data[2]);
}

TEST(MmapTest, mem_mode) {
    Mmap mm;
    uint32_t value = 12345;
    EXPECT_EQ(RC_SUCCESS, mm.Init(sizeof(value)));
    *(uint32_t*)mm.GetData() = value;
    EXPECT_EQ(value, *(uint32_t*)mm.GetData());
    EXPECT_EQ(sizeof(value), mm.GetSize());
}
