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

#ifndef _ST_HPC_PPL_COMMON_EVENT_COUNT_H_
#define _ST_HPC_PPL_COMMON_EVENT_COUNT_H_

#include <stdint.h>
#include <atomic>

namespace ppl { namespace common {

/** https://www.1024cores.net/home/lock-free-algorithms/eventcounts */

class EventCount final {
public:
    typedef uint32_t Key;

public:
    EventCount() : val_(0) {
        static_assert(sizeof(val_) == sizeof(uint64_t), "atomic size mismatch");
    }

    Key PrepareWait();
    void CancelWait();
    void CommitWait(Key);
    void NotifyOne();
    void NotifyAll();

    template <typename Predicate>
    void Wait(Predicate&& stop_waiting) {
        if (stop_waiting()) {
            return;
        }

        while (true) {
            auto key = PrepareWait();
            if (stop_waiting()) {
                CancelWait();
                return;
            }
            CommitWait(key);
        }
    }

private:
    // the epoch in the most significant 32 bits and the waiter count in the least significant 32 bits
    std::atomic<uint64_t> val_;

private:
    EventCount(const EventCount&) = delete;
    EventCount(EventCount&&) = delete;
    void operator=(const EventCount&) = delete;
    void operator=(EventCount&&) = delete;
};

}}

#endif
