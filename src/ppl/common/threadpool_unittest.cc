#include "ppl/common/threadpool.h"
#include "gtest/gtest.h"
using namespace std;
using namespace ppl::common;

class TestThreadTask final : public ThreadTask {
public:
    virtual ~TestThreadTask() {
        printf("TestThreadTask destructor is called.\n");
    }
    shared_ptr<ThreadTask> Run() override {
        printf("Hello, world!\n");
        return shared_ptr<ThreadTask>();
    }
};

TEST(ThreadPoolTest, all) {
    ThreadPool tp;

    ASSERT_TRUE(tp.Init(5) == RC_SUCCESS);
    ASSERT_EQ(5, tp.GetThreadNum());
    ASSERT_TRUE(tp.AddTask(make_shared<TestThreadTask>()) == RC_SUCCESS);

    const uint32_t core_list[] = {0, 1};
    ASSERT_TRUE(tp.SetAffinity(0, core_list, 2) == RC_SUCCESS);
}
