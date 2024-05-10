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

#ifndef _ST_HPC_PPL_COMMON_MPSC_QUEUE_H_
#define _ST_HPC_PPL_COMMON_MPSC_QUEUE_H_

#include <atomic>

namespace ppl { namespace common {

/**
   a lock-free queue implementation for multi-producer-single-consumer
   based on
     - https://www.1024cores.net/home/lock-free-algorithms/queues/intrusive-mpsc-node-based-queue
     - https://github.com/grpc/grpc/blob/master/src/core/lib/gprpp/mpscq.h
*/

class MPSCQueue final {
public:
    struct Node {
        std::atomic<Node*> mpsc_next;
    };

public:
    MPSCQueue() : m_head(&m_stub), m_tail(&m_stub) {
        m_stub.mpsc_next.store(nullptr, std::memory_order_relaxed);
    }

    /** @return tells whether queue may be empty or not before inserting `node`. */
    bool Push(Node* node);

    /**
       @param [out] is_empty indicates whether the queue is empty or not.
       @return may be nullptr if insertion is happening and `is_empty` is false.
    */
    Node* Pop(bool* is_empty);

private:
    // l1 cache line size for most CPUs
    static constexpr int CACHELINE_SIZE = 64;

    union {
        Node* m_head;
        char padding[CACHELINE_SIZE];
    };
    std::atomic<Node*> m_tail;
    Node m_stub;

private:
    MPSCQueue(const MPSCQueue&) = delete;
    MPSCQueue(MPSCQueue&&) = delete;
    void operator=(const MPSCQueue&) = delete;
    void operator=(MPSCQueue&&) = delete;
};

}}

#endif
