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
