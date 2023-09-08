#ifndef _ST_HPC_PPL_COMMON_MESSAGE_QUEUE_H_
#define _ST_HPC_PPL_COMMON_MESSAGE_QUEUE_H_

#include <list>

#ifdef _MSC_VER
#include "ppl/common/windows/pthread.h"
#else
#include <pthread.h>
#endif

namespace ppl { namespace common {

template <typename T>
class MessageQueue final {
public:
    MessageQueue() {
        pthread_mutex_init(&mutex_, nullptr);
        pthread_cond_init(&cond_, nullptr);
    }
    ~MessageQueue() {
        pthread_cond_destroy(&cond_);
        pthread_mutex_destroy(&mutex_);
    }
    void Push(const T& item) {
        pthread_mutex_lock(&mutex_);
        items_.push_back(item);
        pthread_cond_signal(&cond_);
        pthread_mutex_unlock(&mutex_);
    }
    T Pop() {
        pthread_mutex_lock(&mutex_);
        while (items_.empty()) {
            pthread_cond_wait(&cond_, &mutex_);
        }
        auto item = items_.front();
        items_.pop_front();
        pthread_mutex_unlock(&mutex_);
        return item;
    }

private:
    pthread_mutex_t mutex_;
    pthread_cond_t cond_;
    std::list<T> items_;

private:
    MessageQueue(const MessageQueue&) = delete;
    void operator=(const MessageQueue&) = delete;
    MessageQueue(MessageQueue&&) = delete;
    void operator=(MessageQueue&&) = delete;
};

}}

#endif
