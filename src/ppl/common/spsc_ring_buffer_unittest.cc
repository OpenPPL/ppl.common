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

#include "ppl/common/spsc_ring_buffer.h"
#include "gtest/gtest.h"
#include <vector>
#include <random>
#include <thread>
#include <utility>

using namespace ppl::common;

void Produce(int32_t n, SPSCRingBuffer<float>* queue, std::vector<float>* pushed_data) {
    std::default_random_engine gen(0); // init with seed 0
    std::uniform_real_distribution<float> dis(0.0, 10);
    for (int i = 0; i < n; ++i) {
        float data = dis(gen);
        while (queue->Push(data) == false) {
            std::this_thread::yield();
        }
        pushed_data->push_back(data);
    }
}

void Consume(int32_t n, SPSCRingBuffer<float>* queue, std::vector<float>* received_data) {
    for (int i = 0; i < n; ++i) {
        float data;
        while (queue->Pop(&data) == false) {
            std::this_thread::yield();
        }
        received_data->push_back(data);
    }
}

TEST(SPSCRingBufferTest, all) {
    int32_t n = 8192;
    SPSCRingBuffer<float> queue(n);
    std::vector<float> pushed_data;
    std::vector<float> received_data;

    std::thread produce_th(Produce, n, &queue, &pushed_data);
    std::thread consume_th(Consume, n, &queue, &received_data);

    produce_th.join();
    consume_th.join();

    // check result
    ASSERT_EQ(pushed_data.size(), received_data.size());
    for (int i = 0; i < n; ++i) {
        ASSERT_FLOAT_EQ(pushed_data[i], received_data[i]);
    }
}

TEST(SPSCRingBufferTest, empty_full) {
    int32_t n = 8192;
    SPSCRingBuffer<float> queue(n);

    ASSERT_EQ(queue.IsEmpty(), true);

    for (int i = 0; i < n; ++i) {
        queue.Push(i);
    }
    ASSERT_EQ(queue.IsFull(), true);
}