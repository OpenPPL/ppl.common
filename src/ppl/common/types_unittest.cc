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

#include "ppl/common/types.h"
#include "gtest/gtest.h"
#include <vector>
using namespace ppl::common;

TEST(TypesTest, all) {
    EXPECT_EQ(1u, GetSizeOfDataType(DATATYPE_UINT8));
    EXPECT_EQ(1u, GetSizeOfDataType(DATATYPE_INT8));
    EXPECT_EQ(2u, GetSizeOfDataType(DATATYPE_UINT16));
    EXPECT_EQ(2u, GetSizeOfDataType(DATATYPE_INT16));
    EXPECT_EQ(4u, GetSizeOfDataType(DATATYPE_UINT32));
    EXPECT_EQ(4u, GetSizeOfDataType(DATATYPE_INT32));
    EXPECT_EQ(8u, GetSizeOfDataType(DATATYPE_UINT64));
    EXPECT_EQ(8u, GetSizeOfDataType(DATATYPE_INT64));
    EXPECT_EQ(2u, GetSizeOfDataType(DATATYPE_FLOAT16));
    EXPECT_EQ(4u, GetSizeOfDataType(DATATYPE_FLOAT32));
    EXPECT_EQ(8u, GetSizeOfDataType(DATATYPE_FLOAT64));

    std::vector<datatype_t> data_types{
        DATATYPE_UNKNOWN, DATATYPE_UINT8, DATATYPE_INT8,    DATATYPE_UINT16,  DATATYPE_INT16,
        DATATYPE_UINT32,  DATATYPE_INT32, DATATYPE_FLOAT16, DATATYPE_FLOAT32, DATATYPE_FLOAT64,
    };
    for (size_t i = 0; i < data_types.size(); ++i) {
        for (size_t j = 0; j < data_types.size(); ++j) {
            if (i == j) {
                continue;
            }
            EXPECT_NE(data_types[i], data_types[j]);
        }
    }
    EXPECT_EQ(2u, alignof(float16_t));
    EXPECT_EQ(2u, sizeof(float16_t));
}
