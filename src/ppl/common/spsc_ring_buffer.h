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

#ifndef _ST_HPC_PPL_COMMON_SPSC_RING_BUFFER_H_
#define _ST_HPC_PPL_COMMON_SPSC_RING_BUFFER_H_

#include <atomic>
#include <cstddef>
#include <vector>

namespace ppl { namespace common {

/**
   a lock-free ring buffer queue implementation for single-producer-single-consumer
   based on
     - https://www.codeproject.com/Articles/43510/Lock-Free-Single-Producer-Single-Consumer-Circular
*/

template <typename T>
class SPSCRingBuffer final {
public:
    SPSCRingBuffer(size_t size) : tail_(0), head_(0), vec_(size + 1) {}

    template <typename ItemType>
    bool Push(ItemType&& item) {
        const auto current_tail = tail_.load(std::memory_order_relaxed);
        const auto next_tail = Increment(current_tail);
        if (next_tail == head_.load(std::memory_order_acquire)) {
            return false;
        }

        vec_[current_tail] = std::forward<ItemType>(item);
        tail_.store(next_tail, std::memory_order_release);
        return true;
    }

    template <typename ItemType>
    bool Pop(ItemType* item) {
        const auto current_head = head_.load(std::memory_order_relaxed);
        if (current_head == tail_.load(std::memory_order_acquire)) {
            return false;
        }

        *item = std::move(vec_[current_head]);
        head_.store(Increment(current_head), std::memory_order_release);
        return true;
    }

    bool IsEmpty() const {
        return (head_.load(std::memory_order_relaxed) == tail_.load(std::memory_order_relaxed));
    }

    bool IsFull() const {
        const auto next_tail = Increment(tail_.load(std::memory_order_relaxed));
        return (next_tail == head_.load(std::memory_order_relaxed));
    }

private:
    size_t Increment(size_t idx) const {
        return (idx + 1) % (vec_.size());
    }

private:
    // l1 cache line size for most CPUs
    static constexpr int CACHELINE_SIZE = 64;

    union {
        std::atomic<size_t> tail_;
        char padding1[CACHELINE_SIZE];
    };
    union {
        std::atomic<size_t> head_;
        char padding2[CACHELINE_SIZE];
    };
    std::vector<T> vec_;
};

}} // namespace ppl::common

#endif
