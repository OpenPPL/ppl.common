#include "ppl/common/sys.h"
#include <string.h>
#include <mutex>
#ifdef _WIN32
#define NOGDI
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <immintrin.h>
#else
#include <errno.h>
#include <signal.h>
#include <setjmp.h>
#include <immintrin.h>
#include <unistd.h>
#include <fcntl.h>
#endif

template <typename... Args>
static inline void supress_unused_warnings(Args &&...) {}

#ifdef __GNUC__
#define GCC_VERSION (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)
#endif

#ifdef __APPLE__
extern "C" void _do_cpuid(int ieax, int iecx, int* rst_reg);
#elif !(defined(_WIN32) || defined(_WIN64))
extern "C" void __do_cpuid(int ieax, int iecx, int* rst_reg);
#endif
static void do_cpuid(int ieax, int iecx, int *eax, int *ebx, int *ecx, int *edx) {
    int iEXXValue[4];
#if defined(_WIN32) || defined(_WIN64)
    __cpuidex(iEXXValue, ieax, iecx);
#elif defined(__APPLE__)
    _do_cpuid(ieax, iecx, iEXXValue);
#else
    __do_cpuid(ieax, iecx, iEXXValue);
#endif
    if (eax) *eax = iEXXValue[0];
    if (ebx) *ebx = iEXXValue[1];
    if (ecx) *ecx = iEXXValue[2];
    if (edx) *edx = iEXXValue[3];
}

#ifdef _WIN32
static int try_run(float(*func)()) {
    __try {
        func();
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        return -1;
    }
    return 0;
}
#else
static jmp_buf exceptJmpBuf;
static struct sigaction exceptOldAct;
static void exceptHandler(int) {
    siglongjmp(exceptJmpBuf, 1);
}
static int try_run(float(*func)()) {
    struct sigaction act;
    memset(&act, 0, sizeof(act));
    act.sa_handler = &exceptHandler;
    sigaction(SIGILL, &act, &exceptOldAct);
    if (0 == sigsetjmp(exceptJmpBuf, 1)) {
        const float value = func();
        sigaction(SIGILL, &exceptOldAct, NULL);
        supress_unused_warnings(value);
        return 0;
    }
    sigaction(SIGILL, &exceptOldAct, NULL);
    return -1;
}
#endif //! non-windows

namespace ppl {
namespace common {
#ifdef _MSC_VER
#pragma optimize("", off)
#else
#pragma GCC push_options
#pragma GCC optimize ("-O0")
#endif

static
#ifndef _MSC_VER
__attribute__((__target__("sse"))) __attribute__((optimize(0)))
#endif
float testIsaSSE()
{
    return 0.0f;
}

static
#ifndef _MSC_VER
__attribute__((__target__("avx"))) __attribute__((optimize(0)))
#endif
float testIsaAVX()
{
    __m256 ymm0, ymm1, ymm2;
#if defined (_WIN64) || defined (_WIN32)
    __declspec(align(64)) float buf[24] = {
#else
    float buf[24] __attribute__((aligned(64))) = {
#endif
        1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f,
        8.0f, 7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f,
        0.0f, 1.0f, 0.0f, 3.0f, 0.0f, 5.0f, 0.0f, 7.0f,
    };
    ymm0 = _mm256_load_ps(buf);
    ymm1 = _mm256_load_ps(buf + 8);
    ymm1 = _mm256_mul_ps(ymm0, ymm1);
    ymm2 = _mm256_load_ps(buf + 16);
    ymm2 = _mm256_add_ps(ymm1, ymm2);
    _mm256_store_ps(buf + 16, ymm2);
    return buf[18];
}

static
#ifndef _MSC_VER
__attribute__((__target__("fma"))) __attribute__((optimize(0)))
#endif
float testIsaFMA()
{
    __m256 ymm0, ymm1, ymm2;
#if defined (_WIN64) || defined (_WIN32)
    __declspec(align(64)) float buf[24] = {
#else
    float buf[24] __attribute__((aligned(64))) = {
#endif
        1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f,
        8.0f, 7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f,
        0.0f, 1.0f, 0.0f, 3.0f, 0.0f, 5.0f, 0.0f, 7.0f,
    };
    ymm0 = _mm256_load_ps(buf);
    ymm1 = _mm256_load_ps(buf + 8);
    ymm2 = _mm256_load_ps(buf + 16);
    ymm2 = _mm256_fmadd_ps(ymm0, ymm1, ymm2);
    _mm256_store_ps(buf + 16, ymm2);
    return buf[19];
}

// About __AVX512F__ see: https://docs.microsoft.com/en-us/cpp/build/reference/arch-x64?view=vs-2019
// AVX512 was landed in VS2017 and GCC-4.9.2
#if (GCC_VERSION >= 40902 || _MSC_VER >= 1910)
static float
#ifdef __GNUC__
__attribute__((__target__("avx512f"))) __attribute__((optimize(0)))
#endif // __GNUC__
testIsaAVX512() {
    __m512 zmm0, zmm1, zmm2;
#if defined (_WIN64) || defined (_WIN32)
    __declspec(align(64)) float buf[48] = {
#else
    float buf[48] __attribute__((aligned(64))) = {
#endif
        1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f,
        8.0f, 7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f,
        0.0f, 1.0f, 0.0f, 3.0f, 0.0f, 5.0f, 0.0f, 7.0f,
        7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 3.0f, 0.0f, 5.0f, 0.0f, 7.0f,
        7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f, 0.0f,
    };
    zmm0 = _mm512_load_ps(buf);
    zmm1 = _mm512_load_ps(buf + 16);
    zmm2 = _mm512_load_ps(buf + 32);
    zmm2 = _mm512_fmadd_ps(zmm0, zmm1, zmm2);
    _mm512_store_ps(buf + 32, zmm2);
    return buf[41];
}
#endif // __AVX512F__

#ifdef _MSC_VER
#pragma optimize("", on)
#else
#pragma GCC pop_options
#endif

//static int ReadCpuCacheLevelInfoFromSYSFS(int core, int index)
//{
//    char cacheInfoFile[256];
//    snprintf(cacheInfoFile, 256, "/sys/devices/system/cpu/cpu%d/cache/index%d/level", core, index);
//    FILE* cacheLevelFile = fopen(cacheInfoFile, "rb");
//    if (cacheLevelFile == nullptr) {
//        return INT_MAX;
//    }
//    int level = 0;
//    scanf(cacheLevelFile, "%d", &level);
//    fclose(cacheLevelFile);
//    return level;
//}
//static ReadCpuCacheInfoFromSYSFS() {
//    char cacheInfoFile[256];
//    for (int i = 0; i < 128; ++i) {
//        int cacheLevelFile = open("/sys/devices/system/cpu/cpu0/cache/index0/level", O_RDONLY);
//        if (cacheLevelFile == -1) {
//        }
//        read(cacheLevelFile, buffer, 128);
//        close(cacheLevelFile);
//    }
//}
//static constexpr size_t MAX_LINE_SIZE = 4096;
//static constexpr size_t MAX_CPUKEY_SIZE = 64;
//static void ReadCpuInfoFromKVFS() {
//    int cpuinfoFile = open("/proc/cpuinfo", O_RDONLY);
//    if (cpuinfoFile == -1) {
//        return;
//    }
//    char line_buffer[MAX_LINE_SIZE];
//}
//}

static void GetCacheInfo(const int &eax, const int &ebx, const int &ecx, const int &edx,
                        size_t* cache_size, bool* inclusive) {
    int cache_type = eax & 31;
    if (cache_type == 0) {
        *cache_size = 0;
        if (inclusive != nullptr) {
            *inclusive = false;
        }
    } else {
        // int cache_level = (eax >> 5) & 0x7;
        int cache_sets = ecx + 1;
        int cacheline_size = (ebx & 0xfff) + 1;
        int cacheline_partitions = ((ebx >> 12) & 0x3ff) + 1;
        int cache_ways = ((ebx >> 22) & 0x3ff) + 1;
        *cache_size = static_cast<size_t>(cache_ways * cacheline_partitions * cacheline_size * cache_sets);
        if (inclusive != nullptr) {
            *inclusive = (edx >> 1) & 1;
        }
    }
}


void GetCPUInfoByCPUID(struct CpuInfo* info) {
#define BIT_TEST(bit_map, pos) (((bit_map) & (0x1 << (pos))) ? 1 : 0)
    int eax, ebx, ecx, edx;
    do_cpuid(0x1, 0x0, &eax, &ebx, &ecx, &edx);
    info->isa = 0;
    info->isa |= (BIT_TEST(edx, 25) ? 0x1UL : 0x0UL);   // ISA_X86_SSE
    info->isa |= (BIT_TEST(edx, 26) ? 0x2UL : 0x0UL);   // ISA_X86_SSE2
    info->isa |= (BIT_TEST(ecx, 0) ? 0x4UL : 0x0UL);    // ISA_X86_SSE3
    info->isa |= (BIT_TEST(ecx, 9) ? 0x8UL : 0x0UL);    // ISA_X86_SSSE3
    info->isa |= (BIT_TEST(ecx, 19) ? 0x10UL : 0x0UL);  // ISA_X86_SSE41
    info->isa |= (BIT_TEST(ecx, 20) ? 0x20UL : 0x0UL);  // ISA_X86_SSE42
    info->isa |= (BIT_TEST(ecx, 28) ? 0x40UL : 0x0UL);  // ISA_X86_AVX
    info->isa |= (BIT_TEST(ecx, 12) ? 0x100UL : 0x0UL); // ISA_X86_FMA
    do_cpuid(0x7, 0x0, &eax, &ebx, &ecx, &edx);
    info->isa |= (BIT_TEST(ebx, 5) ? 0x80UL : 0x0UL);   // ISA_X86_AVX2
#undef BIT_TEST
    // detect cache
    do_cpuid(4, 0, &eax, &ebx, &ecx, &edx);
    GetCacheInfo(eax, ebx, ecx, edx, &(info->l1_cache_size), nullptr);
    do_cpuid(4, 2, &eax, &ebx, &ecx, &edx);
    GetCacheInfo(eax, ebx, ecx, edx, &(info->l2_cache_size), nullptr);
    do_cpuid(4, 3, &eax, &ebx, &ecx, &edx);
    GetCacheInfo(eax, ebx, ecx, edx, &(info->l3_cache_size), nullptr);
}

void GetCPUInfoByRun(CpuInfo* info) {
#if (GCC_VERSION >= 40902 || _MSC_VER >= 1910)
    if (0 == try_run(&testIsaAVX512)) {
        info->isa |= ISA_X86_AVX512;
    }
#endif // __AVX512F__
    if (0 == try_run(&testIsaFMA)) {
        info->isa |= ISA_X86_FMA;
    }
    if (0 == try_run(&testIsaAVX)) {
        info->isa |= ISA_X86_AVX;
    }
    if (0 == try_run(&testIsaSSE)) {
        info->isa |= ISA_X86_SSE;
    }
    //int eax, ebx, ecx, edx;
    //int iValues[4];
    //do_cpuid(4, 0, &eax, &ebx, &ecx, &edx, iValues);
    //
    // detect cache
    int eax, ebx, ecx, edx;
    do_cpuid(4, 0, &eax, &ebx, &ecx, &edx);
    GetCacheInfo(eax, ebx, ecx, edx, &(info->l1_cache_size), nullptr);
    do_cpuid(4, 2, &eax, &ebx, &ecx, &edx);
    GetCacheInfo(eax, ebx, ecx, edx, &(info->l2_cache_size), nullptr);
    do_cpuid(4, 3, &eax, &ebx, &ecx, &edx);
    GetCacheInfo(eax, ebx, ecx, edx, &(info->l3_cache_size), nullptr);
}

static CpuInfo __st_cpuinfo {0, 0, 0, 0};
static std::once_flag __st_cpuinfo_once_flag;

static void detect_cpuinfo_once() {
    GetCPUInfoByRun(&__st_cpuinfo);
}

const CpuInfo* GetCpuInfo(int) {
    std::call_once(__st_cpuinfo_once_flag, &detect_cpuinfo_once);
    return &__st_cpuinfo;
}
} //! namespace common
} //! namespace ppl

#ifdef __GNUC__
#undef GCC_VERSION
#endif
