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

#ifndef _ST_HPC_PPL_COMMON_CUDA_CUDA_TYPES_H_
#define _ST_HPC_PPL_COMMON_CUDA_CUDA_TYPES_H_

#include <stdint.h>

namespace ppl { namespace common { namespace cuda {

static inline uint32_t GetDataFormatChannelAlignment(dataformat_t dt) {
    static const uint32_t data_format_alignment[] = {
        0, // UNKNOWN
        1, // NDARRAY
        8, // NHWC
        2, // N2CX
        4, // N4CX
        8, // N8CX
        16, // N16CX
        32, // N32CX
    };
    return data_format_alignment[dt];
}

}}} // namespace ppl::common::cuda

#endif
