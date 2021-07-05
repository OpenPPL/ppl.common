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
#include "ppl/common/sys.h"

#ifdef PPLCOMMON_USE_X86
#include <immintrin.h>
#elif defined(PPLCOMMON_USE_ARM)
#include "ppl/common/arm/fp16fp32.h"
#include <arm_neon.h>
#endif

namespace ppl { namespace common {

#if defined(PPLCOMMON_USE_X86)

#if defined(_MSC_VER)
float half2float(uint16_t half) {
    uint16_t src[] = {
        half, 0, 0, 0, 0, 0, 0, 0,
    };
    float dst[4];

    __m128i half_vec = _mm_loadu_si128((const __m128i*)src);
    __m128 float_vec = _mm_cvtph_ps(half_vec);
    _mm_store_ps(dst, float_vec);
    return dst[0];
}
uint16_t float2half(float value) {
    uint16_t dst[8];
    __m128 float_vec = _mm_load_ps1(&value);
    __m128i half_vec = _mm_cvtps_ph(float_vec, 0);
    _mm_storeu_si128((__m128i*)dst, half_vec);
    return dst[0];
}
#else
float half2float(uint16_t half) {
    // if (is_cpu_support(ISA_X86_F16C)) {
    return _cvtsh_ss(half);
    //}
    // else {
    //    return half2float_soft(half);
    //}
}
uint16_t float2half(float value) {
    // if (is_cpu_support(ISA_X86_F16C)) {
    return _cvtss_sh(value, 0);
    //}
    // else {
    //    return float2half_soft(value);
    //}
}
#endif // _MSC_VER

void ConvertFp32ToFp16(const void* fp32, void* fp16, size_t count) {
    size_t i;
    for (i = 0; i < (count / 8) * 8; i += 8) {
        //__m128 x = _mm_loadu_ps((float*)fp32 + i);
        //__m128i y = _mm_cvtps_ph(x, 0);
        //_mm_mask_storeu_epi64((float16_t*)fp16 + i, -1, y);
        //_mm_store1_si64()
        __m256 x = _mm256_loadu_ps((float*)fp32 + i);
        __m128i y = _mm256_cvtps_ph(x, 0);
        _mm_storeu_si128((__m128i*)((float16_t*)fp16 + i), y);
    }
    if (i == count) {
        return;
    }
    float tmp[8] = {};
    float16_t tmpFP16[8] = {};
    for (size_t j = i, k = 0; j < count; ++j, ++k) {
        tmp[k] = ((float*)fp32)[j];
    }
    __m256 x = _mm256_loadu_ps(tmp);
    _mm_storeu_si128((__m128i*)&tmpFP16[0], _mm256_cvtps_ph(x, 0));
    for (size_t k = 0; i < count; ++i, ++k) {
        ((float16_t*)fp16)[i] = tmpFP16[k];
    }
}

void ConvertFp16ToFp32(const void* fp16, void* fp32, size_t count) {
    size_t i;
    for (i = 0; i < (count / 8) * 8; i += 8) {
        __m128i x = _mm_loadu_si128((__m128i*)((float16_t*)fp16 + i));
        __m256 y = _mm256_cvtph_ps(x);
        _mm256_storeu_ps((float*)fp32 + i, y);
    }
    if (i == count) {
        return;
    }
    float tmpFP32[8] = {};
    float16_t tmpFP16[8] = {};
    for (size_t j = i, k = 0; j < count; ++j, ++k) {
        tmpFP16[k] = ((float16_t*)fp16)[j];
    }
    __m128i x = _mm_loadu_si128((__m128i*)&tmpFP16[0]);
    _mm256_storeu_ps(tmpFP32, _mm256_cvtph_ps(x));
    for (size_t j = i, k = 0; j < count; ++j, ++k) {
        ((float*)fp32)[j] = tmpFP32[k];
    }
}
#elif defined(PPLCOMMON_USE_ARM)
float half2float(uint16_t half) {
    uint16_t f16[8] = {half};
    float f32[8];
    ppl3CoreArmFp16fp32_asm(8, f16, f32);
    return f32[0];
}
uint16_t float2half(float value) {
    float f32[8] = {value, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
    uint16_t f16[8];
    ppl3CoreArmFp32fp16_asm(8, f32, f16);
    return f16[0];
}
void ConvertFp32ToFp16(const void* f32, void* f16, size_t count) {
    unsigned short* fp16 = reinterpret_cast<unsigned short*>(f16);
    float const* fp32 = reinterpret_cast<float const*>(f32);
    size_t n8 = count & (~7);
    if (n8 != 0) {
        ppl3CoreArmFp32fp16_asm(n8, fp32, fp16);
    }
    size_t n7 = count - n8;

    if (n7 != 0) {
        unsigned short fp16_tmp[8] __attribute__((aligned(16))) = {0, 0, 0, 0, 0, 0, 0, 0};
        float fp32_tmp[8] __attribute__((aligned(16))) = {0.0f, 0.f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};

        for (size_t i = 0; i < n7; ++i) {
            fp32_tmp[i] = fp32[n8 + i];
        }

        ppl3CoreArmFp32fp16_asm(8, fp32_tmp, fp16_tmp);

        for (size_t i = 0; i < n7; ++i) {
            fp16[n8 + i] = fp16_tmp[i];
        }
    }
}
void ConvertFp16ToFp32(const void* f16, void* f32, size_t count) {
    unsigned short const* fp16 = reinterpret_cast<unsigned short const*>(f16);
    float* fp32 = reinterpret_cast<float*>(f32);
    size_t n8 = count & (~7);
    if (n8 != 0) {
        ppl3CoreArmFp16fp32_asm(n8, fp16, fp32);
    }
    size_t n7 = count - n8;

    if (n7) {
        unsigned short fp16_tmp[8] __attribute__((aligned(16))) = {0, 0, 0, 0, 0, 0, 0, 0};
        float fp32_tmp[8] __attribute__((aligned(16))) = {0.0f, 0.f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};

        for (size_t i = 0; i < n7; ++i) {
            fp16_tmp[i] = fp16[n8 + i];
        }

        ppl3CoreArmFp16fp32_asm(8, fp16_tmp, fp32_tmp);

        for (size_t i = 0; i < n7; ++i) {
            fp32[n8 + i] = fp32_tmp[i];
        }
    }
}
#endif

}} // namespace ppl::common
