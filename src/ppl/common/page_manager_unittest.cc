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

TEST(PageManagerTest, all) {
    PageManager page_manager;
    page_manager.Init(16);

    RetCode ret;
    vector<uint64_t> page_list1;
    ret = page_manager.Alloc(2, &page_list1);
    ASSERT_EQ(RC_SUCCESS, ret);

    vector<uint64_t> page_list2;
    ret = page_manager.Alloc(3, &page_list2);
    ASSERT_EQ(RC_SUCCESS, ret);

    vector<uint64_t> page_list3;
    ret = page_manager.Alloc(4, &page_list3);
    ASSERT_EQ(RC_SUCCESS, ret);

    vector<uint64_t> page_list4;
    ret = page_manager.Alloc(6, &page_list4);
    ASSERT_EQ(RC_SUCCESS, ret);

    vector<uint64_t> page_list5;
    ret = page_manager.Alloc(2, &page_list5);
    ASSERT_EQ(RC_OUT_OF_MEMORY, ret);
    ASSERT_EQ(1, page_manager.GetAvail());

    vector<uint64_t> page_list6;
    page_manager.Free(page_list2.data(), page_list2.size());
    page_manager.Free(page_list3.data(), page_list3.size());

    vector<uint64_t> page_list7;
    ret = page_manager.Alloc(7, &page_list7);
    ASSERT_EQ(std::vector<uint64_t>({2, 3, 4, 5, 6, 7, 8}), page_list7);
}
