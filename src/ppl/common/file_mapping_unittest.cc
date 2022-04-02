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

#include "ppl/common/file_mapping.h"
#include "gtest/gtest.h"
using namespace ppl::common;

TEST(FileMappingTest, init) {
    FileMapping fm;
    EXPECT_EQ(RC_SUCCESS, fm.Init(__FILE__));
    EXPECT_NE(RC_SUCCESS, fm.Init("a"));
}

TEST(FileMappingTest, data_and_size) {
    FileMapping fm;
    EXPECT_EQ(RC_SUCCESS, fm.Init(__FILE__));
    EXPECT_TRUE(fm.Size() > 0);
    EXPECT_NE(nullptr, fm.Data());
}

TEST(FileMappingTest, offset_and_length) {
    FileMapping fm;
    const uint64_t offset = 22;
    const uint64_t length = 14;
    EXPECT_EQ(RC_SUCCESS, fm.Init(__FILE__, offset, length));
    EXPECT_EQ(14, fm.Size());
    EXPECT_NE(nullptr, fm.Data());

    auto data = fm.Data();
    EXPECT_EQ('c', data[0]);
    EXPECT_EQ('h', data[1]);
    EXPECT_EQ('e', data[2]);
}
