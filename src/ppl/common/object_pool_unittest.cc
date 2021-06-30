#include "ppl/common/object_pool.h"
#include "gtest/gtest.h"
using namespace ppl::common;

class TestClass final {
public:
    TestClass(int v) : value_(v) {}
    int GetValue() const {
        return value_;
    }

private:
    int value_;
};

TEST(ObjectPoolTest, alloc_and_free) {
    ObjectPool<TestClass> pool;
    const int value = 100;
    auto c = pool.Alloc(value);
    EXPECT_EQ(value, c->GetValue());
    pool.Free(c);
}
