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
        counter_ = 0;
        pthread_mutex_init(&lock_, nullptr);
        pthread_cond_init(&cond_, nullptr);
    }
    ~Barrier() {
        pthread_cond_destroy(&cond_);
        pthread_mutex_destroy(&lock_);
    }
    void ResetCounter(uint32_t count) {
        counter_ = count;
    }
    void Wait() {
        pthread_mutex_lock(&lock_);
        if (counter_ > 0) {
            --counter_;
            if (counter_ > 0) {
                pthread_cond_wait(&cond_, &lock_);
            } else {
                pthread_cond_broadcast(&cond_);
            }
        }
        pthread_mutex_unlock(&lock_);
    }

private:
    uint32_t counter_;
    pthread_mutex_t lock_;
    pthread_cond_t cond_;
};

}}

#endif
