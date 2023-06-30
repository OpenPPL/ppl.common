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

#ifndef _ST_HPC_PPL_COMMON_CUDA_NCCL_UTILS_H_
#define _ST_HPC_PPL_COMMON_CUDA_NCCL_UTILS_H_

#include <cuda_runtime.h>

#ifdef PPLCOMMON_ENABLE_NCCL
#include <nccl.h>
#endif

#include "ppl/common/log.h"
#include "ppl/common/retcode.h"

#ifdef PPLCOMMON_ENABLE_NCCL
#define NCCL_CHECK(cmd, emsg) \
do {\
    ncclResult_t e = (cmd);\
    if (e != ncclSuccess) {\
        LOG(ERROR) << "NCCL error(code:" << (int)e << ") on " << (emsg);\
        return RC_OTHER_ERROR;\
    }\
} while (0);
#endif

namespace ppl { namespace common {

struct NcclParam {
    int rank{0};
    int size{1};
#ifdef PPLCOMMON_ENABLE_NCCL
    ncclComm_t comm{nullptr};
#endif
};

#ifdef PPLCOMMON_ENABLE_NCCL
template<typename T>
RetCode GetNcclDataType(ncclDataType_t *nccl_data_type)
{
    if (std::is_same<T, float>::value) {
        *nccl_data_type = ncclFloat;
    }
    else if (std::is_same<T, half>::value) {
        *nccl_data_type = ncclHalf;
    }
#if defined(__CUDA_BF16_TYPES_EXIST__)
    else if (std::is_same<T, __nv_bfloat16>::value) {
        *nccl_data_type = ncclBfloat16;
    }
#endif
    else if (std::is_same<T, int>::value) {
        *nccl_data_type = ncclInt;
    }
    else if (std::is_same<T, char>::value) {
        *nccl_data_type = ncclChar;
    }
    else if (std::is_same<T, bool>::value) {
        *nccl_data_type = ncclInt8;
    }
    else {
        LOG(ERROR) << "NCCL only support float, half, bfloat16, int, char, and bool.";
        return RC_INVALID_VALUE;
    }
    return RC_SUCCESS;
}
#endif

template<typename T>
RetCode NcclAllReduceSum(const T* send_buf, T* recv_buf, const int data_size, NcclParam *nccl_param, cudaStream_t stream)
{
#ifdef PPLCOMMON_ENABLE_NCCL
    ncclDataType_t nccl_data_type;
    RetCode r = GetNcclDataType<T>(&nccl_data_type);
    if (RC_SUCCESS != r)
        return r;
    NCCL_CHECK(ncclGroupStart(), "ncclGroupStart");
    NCCL_CHECK(ncclAllReduce(
        (const void*)send_buf, (void*)recv_buf, data_size, nccl_data_type, ncclSum, nccl_param->comm, stream), "ncclAllReduce");
    NCCL_CHECK(ncclGroupEnd(), "ncclGroupEnd");
#endif
    return RC_SUCCESS;
}

template<typename T>
RetCode NcclAllGather(const T* send_buf, T* recv_buf, const int data_size, NcclParam *nccl_param, cudaStream_t stream)
{
#ifdef PPLCOMMON_ENABLE_NCCL
    ncclDataType_t nccl_data_type;
    RetCode r = GetNcclDataType<T>(&nccl_data_type);
    if (RC_SUCCESS != r)
        return r;
    NCCL_CHECK(ncclGroupStart(), "ncclGroupStart");
    NCCL_CHECK(
        ncclAllGather(send_buf, recv_buf, data_size, nccl_data_type, nccl_param->comm, stream), "ncclAllGather");
    NCCL_CHECK(ncclGroupEnd(), "ncclGroupEnd");
#endif
    return RC_SUCCESS;
}

}} // namespace ppl::common

#endif
