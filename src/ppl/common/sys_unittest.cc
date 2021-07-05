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

#include "ppl/common/sys.h"
#include <gtest/gtest.h>
#include <stdio.h>

#ifdef PPLCOMMON_USE_X86
TEST(pplcommon, x86SysInfo) {
    using namespace ppl::common;
    fprintf(stdout, "[x86SysInfo] 0x%lX\n", GetCpuInfo()->isa);
}
#endif //! PPLCOMMON_USE_X86

#define IS_ALIGNED(p, alignment) ((uintptr_t)p % (uintptr_t)alignment == 0)

TEST(pplcommon, AlignedAlloc) {
    void* p0 = ppl::common::AlignedAlloc(100, 64);
    EXPECT_TRUE(IS_ALIGNED(p0, 64));
    char* cp = (char*)p0;
    for (int i = 0; i < 100; ++i) {
        cp[i] = i;
    }
    ppl::common::AlignedFree(p0);
}
