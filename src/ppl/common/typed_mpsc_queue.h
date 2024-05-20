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

#ifndef _ST_HPC_PPL_COMMON_TYPED_MPSC_QUEUE_H_
#define _ST_HPC_PPL_COMMON_TYPED_MPSC_QUEUE_H_

#include "mpsc_queue.h"
#include <atomic>
#include <new>

namespace ppl { namespace common {

// a wrapper for the multi-producer-single-consumer lockless queue

template <typename T>
class TypedMPSCQueue final {
public:
    TypedMPSCQueue() : size_(0) {}

    ~TypedMPSCQueue() {
        bool is_empty;
        MPSCQueue::Node* node;

        while (true) {
            node = queue_.Pop(&is_empty);
            if (!node) {
                return;
            }
            delete static_cast<Item*>(node);
        }
    }

    template <typename ValueType>
    ppl::common::RetCode Push(ValueType&& value) {
        auto item = new (std::nothrow) Item(std::forward<T>(value));
        if (!item) {
            return ppl::common::RC_OUT_OF_MEMORY;
        }

        queue_.Push(item);
        size_.fetch_add(1, std::memory_order_relaxed);

        return ppl::common::RC_SUCCESS;
    }

    template <typename ValueType>
    bool Pop(ValueType* res) {
        bool is_empty = true;
        MPSCQueue::Node* node;
        do {
            node = queue_.Pop(&is_empty);
        } while (!node && !is_empty);

        if (is_empty) {
            return false;
        }

        size_.fetch_sub(1, std::memory_order_relaxed);

        auto item = static_cast<Item*>(node);
        *res = std::move(item->value);
        delete item;

        return true;
    }

    // approximate size
    uint32_t Size() const {
        return size_.load(std::memory_order_relaxed);
    }

private:
    struct Item final : public MPSCQueue::Node {
        Item(T&& v) : value(std::forward<T>(v)) {}
        T value;
    };

private:
    MPSCQueue queue_;
    std::atomic<uint32_t> size_;

private:
    TypedMPSCQueue(const TypedMPSCQueue&) = delete;
    TypedMPSCQueue(TypedMPSCQueue&&) = delete;
    void operator=(const TypedMPSCQueue&) = delete;
    void operator=(TypedMPSCQueue&&) = delete;
};

}}

#endif
