#ifndef _ST_HPC_PPL_COMMON_THREADPOOL_H_
#define _ST_HPC_PPL_COMMON_THREADPOOL_H_

#ifdef _MSC_VER
#include "ppl/common/windows/pthread.h"
#else
#include <pthread.h>
#endif

namespace ppl { namespace common {

class Barrier final {
public:
    Barrier() {
        pthread_mutex_init(&lock_, nullptr);
        pthread_cond_init(&cond_, nullptr);
    }
    ~Barrier() {
        pthread_cond_destroy(&cond_);
        pthread_mutex_destroy(&lock_);
    }

    Retcode Init(uint32_t max_count) {
        max_count_ = max_count;
        counter_ = 0;
    }
    
    void Wait() {
        pthread_mutex_lock(&lock_);
        ++counter_;
        if (counter_ < max_count_) {
            pthread_cond_wait(&cond_, &lock_);
        } else {
            counter_ = 0;
            pthread_cond_broadcast(&cond_);
        }
        pthread_mutex_unlock(&lock_);
    }

private:
    uint32_t counter_;
    uint32_t max_count_;
    pthread_mutex_t lock_;
    pthread_cond_t cond_;
};

}}

#endif
