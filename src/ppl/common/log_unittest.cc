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

#include "ppl/common/log.h"
#include "gtest/gtest.h"
using namespace ppl::common;

TEST(LogTest, int_type) {
    int8_t i8 = 1;
    uint8_t u8 = 2;
    int16_t i16 = 3;
    uint16_t u16 = 4;
    int32_t i32 = 5;
    uint32_t u32 = 6;
    int64_t i64 = 7;
    uint64_t u64 = 8;
    float f = 9.0;
    double d = 10.0;

    LOG(DEBUG) << i8 << u8 << i16 << u16 << i32 << u32 << i64 << u64 << f << d;
}
