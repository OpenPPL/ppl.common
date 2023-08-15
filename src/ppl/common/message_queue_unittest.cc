#include "ppl/common/message_queue.h"
#include "gtest/gtest.h"
using namespace ppl::common;

TEST(MessageQueueTest, all) {
    MessageQueue<int> mq;
    mq.Push(1);
    mq.Push(5);
    ASSERT_EQ(1, mq.Pop());
    ASSERT_EQ(5, mq.Pop());
}
