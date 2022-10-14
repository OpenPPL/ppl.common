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

#if defined(PPLCOMMON_USE_X86) || defined(PPLCOMMON_USE_ARM)

#include "ppl/common/half.h"
#include <benchmark/benchmark.h>

static void BM_fp32tofp16(benchmark::State& state) {
    float* f32 = new float[state.range(0)];
    ppl::common::float16_t* f16 = new ppl::common::float16_t[state.range(0)];
    for (auto _ : state) {
        ppl::common::ConvertFp32ToFp16(f32, f16, state.range(0));
    }
    state.SetItemsProcessed(state.iterations() * state.range(0));
    state.SetBytesProcessed(state.items_processed() * sizeof(float));
    delete[] f16;
    delete[] f32;
}

static void BM_fp16tofp32(benchmark::State& state) {
    float* f32 = new float[state.range(0)];
    ppl::common::float16_t* f16 = new ppl::common::float16_t[state.range(0)];
    for (auto _ : state) {
        ppl::common::ConvertFp16ToFp32(f16, f32, state.range(0));
    }
    state.SetItemsProcessed(state.iterations() * state.range(0));
    state.SetBytesProcessed(state.items_processed() * sizeof(ppl::common::float16_t));
    delete[] f16;
    delete[] f32;
}

BENCHMARK(BM_fp32tofp16)->Arg(1)->Arg(2)->Arg(3)->Arg(4)->Arg(5)->RangeMultiplier(2)->Range(1 << 6, 1 << 12);
BENCHMARK(BM_fp16tofp32)->Arg(1)->Arg(2)->Arg(3)->Arg(4)->Arg(5)->RangeMultiplier(2)->Range(1 << 6, 1 << 12);

static void BM_fp32tofp16_single_soft(benchmark::State& state) {
    float* f32 = new float[state.range(0)];
    ppl::common::float16_t* f16 = new ppl::common::float16_t[state.range(0)];

    for (auto _ : state) {
        for (int i = 0; i < state.range(0); ++i) {
            f16[i] = ppl::common::float16_t::FromRawU16(ppl::common::float2half_soft(f32[i]));
        }
    }
    state.SetItemsProcessed(state.iterations() * state.range(0));
    state.SetBytesProcessed(state.items_processed() * sizeof(float));

    delete[] f16;
    delete[] f32;
}

static void BM_fp16tofp32_single_soft(benchmark::State& state) {
    float* f32 = new float[state.range(0)];
    ppl::common::float16_t* f16 = new ppl::common::float16_t[state.range(0)];

    for (auto _ : state) {
        for (int i = 0; i < state.range(0); ++i) {
            f32[i] = ppl::common::half2float_soft(f16[i].Raw16());
        }
    }
    state.SetItemsProcessed(state.iterations() * state.range(0));
    state.SetBytesProcessed(state.items_processed() * 2);

    delete[] f16;
    delete[] f32;
}

BENCHMARK(BM_fp32tofp16_single_soft)->Arg(1);
BENCHMARK(BM_fp16tofp32_single_soft)->Arg(1);

static void BM_fp32tofp16_single(benchmark::State& state) {
    float* f32 = new float[state.range(0)];
    ppl::common::float16_t* f16 = new ppl::common::float16_t[state.range(0)];

    for (auto _ : state) {
        for (int i = 0; i < state.range(0); ++i) {
            f16[i] = ppl::common::float16_t::FromRawU16(ppl::common::float2half(f32[i]));
        }
    }
    state.SetItemsProcessed(state.iterations() * state.range(0));
    state.SetBytesProcessed(state.items_processed() * sizeof(float));

    delete[] f16;
    delete[] f32;
}

static void BM_fp16tofp32_single(benchmark::State& state) {
    float* f32 = new float[state.range(0)];
    ppl::common::float16_t* f16 = new ppl::common::float16_t[state.range(0)];

    for (auto _ : state) {
        for (int i = 0; i < state.range(0); ++i) {
            f32[i] = ppl::common::half2float(f16[i].Raw16());
        }
    }
    state.SetItemsProcessed(state.iterations() * state.range(0));
    state.SetBytesProcessed(state.items_processed() * 2);

    delete[] f16;
    delete[] f32;
}

BENCHMARK(BM_fp32tofp16_single)->Arg(1);
BENCHMARK(BM_fp16tofp32_single)->Arg(1);

#endif
