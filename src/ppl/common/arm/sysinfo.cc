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
#include "ppl/common/arm/sysinfo.h"
#include <mutex>
#include <cstring>
#include <vector>
#include <string>
#include <fstream>
#include <arm_neon.h>

#include <iostream>

#ifdef _WIN32
#include <windows.h>
#include <immintrin.h>
#else
#include <errno.h>
#include <signal.h>
#include <setjmp.h>
#endif

template <typename... Args>
static inline void supress_unused_warnings(Args &&...) {}

#ifdef _WIN32
static int try_run(void(*func)()) {
    __try {
        func();
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        return -1;
    }
    return 0;
}
#else   // for posix
static jmp_buf exceptJmpBuf;
static struct sigaction exceptOldAct;
static void exceptHandler(int) {
    siglongjmp(exceptJmpBuf, 1);
}
static int try_run(void(*func)()) {
    struct sigaction act;
    memset(&act, 0, sizeof(act));
    act.sa_handler = &exceptHandler;
    sigaction(SIGILL, &act, &exceptOldAct);
    if (0 == sigsetjmp(exceptJmpBuf, 1)) {
        func();
        sigaction(SIGILL, &exceptOldAct, NULL);
        return 0;
    }
    sigaction(SIGILL, &exceptOldAct, NULL);
    return -1;
}
#endif //! non-windows

namespace ppl { namespace common {

#if defined(__aarch64__) && defined(PPLCOMMON_USE_ARMV8_2)
static void TestISASupportARMV8_2() {
    asm volatile(
        "fadd   v2.8h,      v0.8h,      v1.8h       \r\n"
        "fmul   v2.8h,      v0.8h,      v1.8h       \r\n"
        "fmax   v2.8h,      v0.8h,      v1.8h       \r\n"
        "fmla   v2.8h,      v0.8h,      v1.8h       \r\n"
        "fmla   v2.8h,      v0.8h,      v1.h[0]     \r\n"
        "fmla   v2.8h,      v0.8h,      v1.h[1]     \r\n"
        "fmla   v2.8h,      v0.8h,      v1.h[2]     \r\n"
        "fmla   v2.8h,      v0.8h,      v1.h[3]     \r\n"
        "fmla   v2.8h,      v0.8h,      v1.h[4]     \r\n"
        "fmla   v2.8h,      v0.8h,      v1.h[5]     \r\n"
        "fmla   v2.8h,      v0.8h,      v1.h[6]     \r\n"
        "fmla   v2.8h,      v0.8h,      v1.h[7]     \r\n"
        :
        :
        :"memory", "cc", "v0", "v1", "v2"
    );
}
#endif

#if defined(__aarch64__) && defined(PPLCOMMON_USE_ARMV8_2_BF16)
static void TestISASupportARMV8_2_BF16() {
    asm volatile(
        "bfmmla  v2.4s,     v0.8h,      v1.8h       \r\n"
        "bfdot   v2.4s,     v0.8h,      v1.8h       \r\n"
        "bfcvtn  v0.4h,     v1.4s                   \r\n"
        "bfcvtn2 v0.8h,     v1.4s                   \r\n"
        :
        :
        :"memory", "cc", "v0", "v1", "v2"
    );
}
#endif

#if defined(__aarch64__) && defined(PPLCOMMON_USE_ARMV8_2_I8MM)
static void TestISASupportARMV8_2_I8MM() {
    asm volatile(
        "smmla   v2.4s,     v0.16b,     v0.16b      \r\n"
        "ummla   v2.4s,     v0.16b,     v0.16b      \r\n"
        "usmmla  v2.4s,     v0.16b,     v0.16b      \r\n"
        :
        :
        :"memory", "cc", "v0", "v1", "v2"
    );
}
#endif

static void GetCPUISAByRun(CpuInfo* info) {
    info->isa = 0;
#ifdef __aarch64__
    info->isa |= ISA_ARMV8;
#endif
#if defined(__aarch64__) && defined(PPLCOMMON_USE_ARMV8_2)
    if (0 == try_run(TestISASupportARMV8_2)) {
        info->isa |= ISA_ARMV8_2;
    }
#endif
#if defined(__aarch64__) && defined(PPLCOMMON_USE_ARMV8_2_BF16)
    if (0 == try_run(TestISASupportARMV8_2_BF16)) {
        info->isa |= ISA_ARMV8_2_BF16;
    }
#endif
#if defined(__aarch64__) && defined(PPLCOMMON_USE_ARMV8_2_I8MM)
    if (0 == try_run(TestISASupportARMV8_2_I8MM)) {
        info->isa |= ISA_ARMV8_2_I8MM;
    }
#endif
}

static inline bool ReadFromFile(const std::string& file, std::string& content) {
    std::ifstream f(file);
    if (!f.is_open()) {
        return false;
    }
    f >> content;
    return f.good();
}

inline uint64_t SizeStrToBytes(const std::string& size_str) {
    if (size_str.empty()) {
        return 0;
    }
    const char suffix = size_str[size_str.size() - 1];
    if (suffix == 'K' or suffix == 'k') {
        return std::stoul(size_str.substr(0, size_str.size() - 1)) * 1024;
    }
    if (suffix == 'M' or suffix == 'm') {
        return std::stoul(size_str.substr(0, size_str.size() - 1)) * 1024 * 1024;
    }
    return std::stoul(size_str);
}

static void GetCacheSizesFromKVFS(CpuInfo* info) {
    info->l1_cache_size = 0;
    info->l2_cache_size = 0;
    info->l3_cache_size = 0;
    const std::string root_dir = "/sys/devices/system/cpu/cpu0/cache/";
    int32_t idx = 0;
    while (1) {
        const std::string index_path = root_dir + "index" + std::to_string(idx) + "/";
        idx++;
        std::string size;
        std::string level;
        std::string type;
        if (!ReadFromFile(index_path + "size", size)) {
            break;
        }
        if (!ReadFromFile(index_path + "level", level)) {
            break;
        }
        if (!ReadFromFile(index_path + "type", type)) {
            break;
        }

        if (type == "Instruction") {
            continue;
        }
        const uint64_t bytes = SizeStrToBytes(size);
        if (level == "1") {
            info->l1_cache_size = bytes;
        } else if (level == "2") {
            info->l2_cache_size = bytes;
        } else if (level == "3") {
            info->l3_cache_size = bytes;
        }
    }
}

static void GetCacheSizes(CpuInfo* info) {
#ifdef _WIN32
#else
    GetCacheSizesFromKVFS(info);
#endif
}

static CpuInfo __st_cpuinfo {0, 0, 0, 0};
static std::once_flag __st_cpuinfo_once_flag;

static void detect_cpuinfo_once(void) {
    GetCPUISAByRun(&__st_cpuinfo);
    GetCacheSizes(&__st_cpuinfo);
}

const CpuInfo* GetCpuInfo(int) {
    std::call_once(__st_cpuinfo_once_flag, &detect_cpuinfo_once);
    return &__st_cpuinfo;
}

}};
