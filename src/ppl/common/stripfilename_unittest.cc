#include "ppl/common/stripfilename.h"
#include <gtest/gtest.h>

TEST(StripFileName, all) {
    EXPECT_STREQ(ppl::common::stripfilename("abc.cc"), "abc.cc");
    EXPECT_STREQ(ppl::common::stripfilename("./abc.cc"), "abc.cc");
    EXPECT_STREQ(ppl::common::stripfilename("./xxx/abc.cc"), "abc.cc");
    EXPECT_STREQ(ppl::common::stripfilename("/xxx/abc.cc"), "abc.cc");
}
