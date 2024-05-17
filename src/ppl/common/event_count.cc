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

#include "event_count.h"
#include "futex_wrapper.h"
using namespace std;

// based on https://github.com/facebook/folly/blob/main/folly/experimental/EventCount.h

namespace ppl { namespace common {

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
static inline uint32_t* GetEpochAddr(uint64_t* v) {
    return reinterpret_cast<uint32_t*>(v) + 1;
}
#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
static inline uint32_t* GetEpochAddr(uint64_t* v) {
    return reinterpret_cast<uint32_t*>(v);
}
#else
#error "unsupported endian format"
#endif

#define EPOCH_SHIFT 32
#define ONE_WAITER 1
#define ONE_EPOCH ((uint64_t)1 << EPOCH_SHIFT)
#define WAITER_MASK ((uint64_t)0xffffffff)

EventCount::Key EventCount::PrepareWait() {
    uint64_t prev = val_.fetch_add(ONE_WAITER, std::memory_order_acq_rel);
    return (prev >> EPOCH_SHIFT);
}

void EventCount::CancelWait() {
    /*
      the faster #waiters gets to 0, the less likely it is that we'll do spurious wakeups
      (and thus system calls).
    */
    val_.fetch_sub(ONE_WAITER, std::memory_order_seq_cst);
}

void EventCount::CommitWait(EventCount::Key v) {
    volatile uint32_t* epoch = GetEpochAddr(reinterpret_cast<uint64_t*>(&val_));
    while (*epoch == v) {
        FutexWait(const_cast<uint32_t*>(epoch), v);
    }
    /*
      the faster #waiters gets to 0, the less likely it is that we'll do spurious wakeups
      (and thus system calls).
    */
    val_.fetch_sub(ONE_WAITER, std::memory_order_seq_cst);
}

void EventCount::NotifyOne() {
    auto prev = val_.fetch_add(ONE_EPOCH, std::memory_order_acq_rel);
    if (prev & WAITER_MASK) {
        FutexWakeOne(GetEpochAddr(reinterpret_cast<uint64_t*>(&val_)));
    }
}

void EventCount::NotifyAll() {
    auto prev = val_.fetch_add(ONE_EPOCH, std::memory_order_acq_rel);
    if (prev & WAITER_MASK) {
        FutexWakeAll(GetEpochAddr(reinterpret_cast<uint64_t*>(&val_)));
    }
}

}}
