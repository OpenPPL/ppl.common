#include "ppl/common/generic_cpu_allocator.h"
#include "gtest/gtest.h"
using namespace ppl::common;

TEST(GenericCpuAllocatorTest, alloc_and_free) {
    GenericCpuAllocator ar(64u);
    auto ret = ar.Alloc(100);
    EXPECT_NE(nullptr, ret);
    ar.Free(ret);
}

TEST(GenericCpuAllocatorTest, alignment) {
    uint32_t expected_alignment = 32;
    GenericCpuAllocator ar(expected_alignment);

    auto ret = ar.Alloc(189);
    EXPECT_NE(nullptr, ret);
    EXPECT_EQ(0, (uintptr_t)ret % expected_alignment);
    ar.Free(ret);
}
