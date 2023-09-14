// Licensed to the Apache Software Foundation (ASF) under one
// or more contributor license agreements.  See the NOTICE file
// distributed with this work for additional information
// regarding copyright ownership.  The ASF licenses this file
// to you under the Apache License, Version 2.0 (the
// "License"); you may not use this file except in compliance
// with the License.  You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing,
// software distributed under the License is distributed on an
// "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied.  See the License for the
// specific language governing permissions and limitations
// under the License.

#ifndef _ST_HPC_PPL_COMMON_BARRIER_H_
#define _ST_HPC_PPL_COMMON_BARRIER_H_

#ifdef _MSC_VER
#include "ppl/common/windows/pthread.h"
#else
#include <pthread.h>
#endif

#include "ppl/common/retcode.h"

namespace ppl { namespace common {

class Barrier final {
public:
    Barrier(uint32_t max_count = 0) : counter_(0), max_count_(max_count) {
        pthread_mutex_init(&lock_, nullptr);
        pthread_cond_init(&cond_, nullptr);
    }
    ~Barrier() {
        pthread_cond_destroy(&cond_);
        pthread_mutex_destroy(&lock_);
    }

    void Reset(uint32_t max_count) {
        max_count_ = max_count;
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
