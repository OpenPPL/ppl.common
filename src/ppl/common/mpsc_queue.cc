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

#include "mpsc_queue.h"
using namespace std;

namespace ppl { namespace common {

bool MPSCQueue::Push(Node* node) {
    node->mpsc_next.store(nullptr, std::memory_order_relaxed);
    auto prev = m_tail.exchange(node, std::memory_order_acq_rel);
    prev->mpsc_next.store(node, std::memory_order_release);
    return (prev == &m_stub);
}

MPSCQueue::Node* MPSCQueue::Pop(bool* is_empty) {
    auto head = m_head;
    auto next = head->mpsc_next.load(std::memory_order_acquire);

    if (head == &m_stub) {
        if (!next) {
            *is_empty = true;
            return nullptr;
        }

        m_head = next;
        head = next;
        next = next->mpsc_next.load(std::memory_order_acquire);
    }

    if (next) {
        m_head = next;
        *is_empty = false;
        return head;
    }

    auto tail = m_tail.load(std::memory_order_acquire);
    if (head != tail) {
        *is_empty = false;
        return nullptr; // inserting
    }

    Push(&m_stub);

    next = head->mpsc_next.load(std::memory_order_acquire);
    if (next) {
        m_head = next;
        *is_empty = false;
        return head;
    }

    *is_empty = false;
    return nullptr; // inserting
}

}}
