#ifndef _ST_HPC_PPL_COMMON_SYS_H_
#define _ST_HPC_PPL_COMMON_SYS_H_

#include <stdint.h>

namespace ppl { namespace common {

/****************************************************
 * @brief ISA
 * represents the instruction set.
 ***************************************************/
enum ISA {
    ISA_undef = 0,
    ISA_X86_SSE = 0x1,
    ISA_X86_SSE2 = 0x2,
    ISA_X86_SSE3 = 0x4,
    ISA_X86_SSSE3 = 0x8,
    ISA_X86_SSE41 = 0x10,
    ISA_X86_SSE42 = 0x20,
    ISA_X86_AVX = 0x40,
    ISA_X86_AVX2 = 0x80,
    ISA_X86_FMA = 0x100,
    ISA_X86_F16C = 0x200,
    ISA_X86_AVX512 = 0x1000,
};
typedef uint32_t isa_t;

struct CpuInfo {
    unsigned long isa;
    uint64_t l1_cache_size;
    uint64_t l2_cache_size;
    uint64_t l3_cache_size;
};

const CpuInfo* GetCpuInfo(int which = 0);

static inline uint64_t GetCpuCacheL1(int which = 0) {
    return GetCpuInfo(which)->l1_cache_size;
}
static inline uint64_t GetCpuCacheL2(int which = 0) {
    return GetCpuInfo(which)->l2_cache_size;
}
static inline uint64_t GetCpuCacheL3(int which = 0) {
    return GetCpuInfo(which)->l3_cache_size;
}

static inline bool CpuSupports(int flag, int which = 0) {
    return GetCpuInfo(which)->isa & flag;
}

static inline uint32_t GetCpuISA(int which = 0) {
    return GetCpuInfo(which)->isa;
}

void GetCPUInfoByCPUID(CpuInfo* info);
void GetCPUInfoByRun(CpuInfo* info);

void* AlignedAlloc(uint64_t size, uint32_t alignment);
void AlignedFree(void* p);

}} // namespace ppl::common

#endif
