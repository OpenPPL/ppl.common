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
#include <string.h>
#include <stdlib.h>
#ifdef _WIN32
#define WIN32_LEAN_MEAN
#define NOGDI // remove ERROR def
#include <malloc.h>
#include <Windows.h>
#endif

namespace ppl { namespace common {

static inline void* AlignedAlloc_impl(uint64_t size, uint32_t alignment) {
#if defined(_WIN32)
    return _aligned_malloc(size, alignment);
#elif defined(__ANDROID__)
    return memalign(alignment, size);
#elif defined(__linux__) || defined(__APPLE__) || defined(__QNX__)
    void* p = nullptr;
    int q = posix_memalign(&p, alignment, size);
    if (q == 0) {
        return p;
    }
    return nullptr;
#else
#error "Unknown platform."
#endif // _WIN32
}

static inline void AlignedFree_impl(void* p) {
#if defined(_WIN32)
    _aligned_free(p);
#else
    free(p);
#endif
}

void* AlignedAlloc(uint64_t size, uint32_t alignment) {
    return AlignedAlloc_impl(size, alignment);
}
void AlignedFree(void* p) {
    AlignedFree_impl(p);
}

}} // namespace ppl::common
