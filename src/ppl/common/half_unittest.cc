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

#include "ppl/common/half.h"
#include <gtest/gtest.h>
#include <stdio.h>
#include <vector>
#include <random>
#include <algorithm>
#include <ctime>
#include <cmath>

static const float FLOAT16_EPS = 5.0e-3f;

TEST(PPLCommon, half_calculation) {
    using namespace ppl::common;
    float16_t x = 0.9f;
    float y = 0.9;
    float z = y + x;
    EXPECT_LT(std::abs(z - 2 * y), FLOAT16_EPS);
}

TEST(PPLCommon, half) {
    const size_t NumberElems = 11111;
    using namespace ppl::common;
    std::default_random_engine eng(std::clock());
    std::uniform_real_distribution<float> distr;
    std::vector<float> fp32Data(NumberElems, 0.0f);
    std::vector<float> fp32Data2(NumberElems, 0.0f);
    std::vector<float16_t> fp16Data(NumberElems);
    for (size_t i = 0; i < NumberElems; ++i) {
        fp32Data[i] = distr(eng);
    }
    ConvertFp32ToFp16(fp32Data.data(), fp16Data.data(), NumberElems);
    ConvertFp16ToFp32(fp16Data.data(), fp32Data2.data(), NumberElems);
    for (size_t i = 0; i < NumberElems; ++i) {
        float v = std::abs(fp32Data2[i] - fp32Data[i]);
        if (v == 0) {
            continue;
        }
        float k = v / std::max(std::abs(fp32Data2[i]), std::abs(fp32Data[i]));
        EXPECT_LE(k, FLOAT16_EPS) << fp32Data[i] << " : " << fp32Data2[i];
    }
}

TEST(PPLCommon, half_soft) {
    const size_t NumberElems = 1111;
    using namespace ppl::common;
    std::default_random_engine eng(std::clock());
    std::uniform_real_distribution<float> distr;
    std::vector<float> fp32Data(NumberElems, 0.0f);
    std::vector<float> fp32Data2(NumberElems, 0.0f);
    std::vector<float16_t> fp16Data(NumberElems);
    for (size_t i = 0; i < NumberElems; ++i) {
        fp32Data[i] = distr(eng);
    }
    for (size_t i = 0; i < NumberElems; ++i) {
        fp16Data[i] = float16_t::FromRawU16(float2half_soft(fp32Data[i]));
    }
    for (size_t i = 0; i < NumberElems; ++i) {
        fp32Data2[i] = half2float_soft(fp16Data[i].bit16);
    }
    for (size_t i = 0; i < NumberElems; ++i) {
        float v = std::abs(fp32Data2[i] - fp32Data[i]);
        if (v == 0) {
            continue;
        }
        float k = v / std::max(std::abs(fp32Data2[i]), std::abs(fp32Data[i]));
        EXPECT_LE(k, FLOAT16_EPS);
    }
}

TEST(PPLCommon, half_hard) {
    const size_t NumberElems = 1111;
    using namespace ppl::common;
    std::default_random_engine eng(std::clock());
    std::uniform_real_distribution<float> distr(-255.0f, 255.0f);
    std::vector<float> fp32Data(NumberElems, 0.0f);
    std::vector<float> fp32Data2(NumberElems, 0.0f);
    std::vector<float16_t> fp16Data(NumberElems);
    for (size_t i = 0; i < NumberElems; ++i) {
        fp32Data[i] = distr(eng);
    }
    for (size_t i = 0; i < NumberElems; ++i) {
        fp16Data[i] = float16_t::FromRawU16(float2half(fp32Data[i]));
    }
    for (size_t i = 0; i < NumberElems; ++i) {
        fp32Data2[i] = half2float(fp16Data[i].bit16);
    }
    for (size_t i = 0; i < NumberElems; ++i) {
        float v = std::abs(fp32Data2[i] - fp32Data[i]);
        if (v == 0) {
            continue;
        }
        float k = v / std::max(std::abs(fp32Data2[i]), std::abs(fp32Data[i]));
        EXPECT_LE(k, FLOAT16_EPS) << fp32Data[i] << " : " << fp32Data2[i];
    }
}

TEST(PPLCommon, half_class) {
    const size_t NumberElems = 11;
    using namespace ppl::common;
    std::default_random_engine eng(std::clock());
    std::uniform_real_distribution<float> distr(-255.0f, 255.0f);
    for (size_t i = 0; i < NumberElems; ++i) {
        float f32 = distr(eng);
        float16_t f16 = f32;
        float v = std::abs(f32 - f16.ToFloat32());
        if (v == 0) {
            continue;
        }
        float k = v / std::max(std::abs(f32), std::abs(f16.ToFloat32()));
        EXPECT_LE(k, FLOAT16_EPS) << f32 << " : " << f16.ToFloat32();
    }
}
