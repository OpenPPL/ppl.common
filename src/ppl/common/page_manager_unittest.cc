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
#include "gtest/gtest.h"

using namespace std;
using namespace ppl::common;

TEST(PageManagerTest, test1) {
    PageManager page_manager;
    page_manager.Init(16 * 8, 8);

    RetCode ret;
    vector<int64_t> page_list1;
    ret = page_manager.Alloc(2, &page_list1);   // [0, 8]
    ASSERT_EQ(RC_SUCCESS, ret);

    vector<int64_t> page_list2;
    ret = page_manager.Alloc(3, &page_list2);   // [16, 24, 32]
    ASSERT_EQ(RC_SUCCESS, ret);

    vector<int64_t> page_list3;
    ret = page_manager.Alloc(4, &page_list3);   // [40, 48, 56, 64]
    ASSERT_EQ(RC_SUCCESS, ret);

    vector<int64_t> page_list4;
    ret = page_manager.Alloc(6, &page_list4);   // [72, 80, 88, 96, 104, 112]
    ASSERT_EQ(RC_SUCCESS, ret);
    ASSERT_EQ(std::vector<int64_t>({72, 80, 88, 96, 104, 112}), page_list4);

    ASSERT_EQ(1, page_manager.GetAvail());

    page_manager.Free(page_list1.data(), page_list1.size());
    page_manager.Free(page_list3.data(), page_list3.size());

    vector<int64_t> page_list5;
    ret = page_manager.Alloc(7, &page_list5);

    ASSERT_EQ(std::vector<int64_t>({40, 48, 56, 64, 0, 8, 120}), page_list5);

    page_manager.Free(page_list2.data(), page_list2.size());
    page_manager.Free(page_list4.data(), page_list4.size());
    page_manager.Free(page_list5.data(), page_list5.size());

    vector<int64_t> page_list6;
    ret = page_manager.Alloc(16, &page_list6);
    ASSERT_EQ(RC_SUCCESS, ret);
    ASSERT_EQ(std::vector<int64_t>({0, 8, 16, 24, 32, 40, 48, 56, 64, 72, 80, 88, 96, 104, 112, 120}), page_list6);

    vector<int64_t> page_list7;
    ret = page_manager.Alloc(1, &page_list7);
    ASSERT_EQ(RC_OUT_OF_MEMORY, ret);

    page_manager.Free(page_list6.data(), page_list6.size());
}

TEST(PageManagerTest, test2) {
    PageManager page_manager;
    page_manager.Init(40 * 8, 8);

    RetCode ret;
    vector<int64_t> page_list1;
    ret = page_manager.Alloc(10, &page_list1);  // [0, 10)
    ASSERT_EQ(RC_SUCCESS, ret);

    vector<int64_t> page_list2;
    ret = page_manager.Alloc(1, &page_list2);   // [10,11)
    ASSERT_EQ(RC_SUCCESS, ret);

    vector<int64_t> page_list3;
    ret = page_manager.Alloc(9, &page_list3);   // [11:20)
    ASSERT_EQ(RC_SUCCESS, ret);

    vector<int64_t> page_list4;
    ret = page_manager.Alloc(2, &page_list4);   // [20,22)
    ASSERT_EQ(RC_SUCCESS, ret);

    vector<int64_t> page_list5;
    ret = page_manager.Alloc(8, &page_list5);   // [22, 30)
    ASSERT_EQ(RC_SUCCESS, ret);

    vector<int64_t> page_list6;
    ret = page_manager.Alloc(2, &page_list6);   // [30, 32)
    ASSERT_EQ(RC_SUCCESS, ret);
    ASSERT_EQ(std::vector<int64_t>({240, 248}), page_list6);

    vector<int64_t> page_list7;
    ret = page_manager.Alloc(8, &page_list7);   // [32, 40)
    ASSERT_EQ(RC_SUCCESS, ret);

    page_manager.Free(page_list2.data(), page_list2.size());
    page_manager.Free(page_list4.data(), page_list4.size());
    page_manager.Free(page_list6.data(), page_list6.size());

    ASSERT_EQ(5, page_manager.GetAvail());

    vector<int64_t> page_list8;
    ret = page_manager.Alloc(2, &page_list8);
    ASSERT_EQ(RC_SUCCESS, ret);
    ASSERT_EQ(std::vector<int64_t>({160, 168}), page_list8);

    ASSERT_EQ(3, page_manager.GetAvail());
}
