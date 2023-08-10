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

#include "cuda_env.h"
#include "ppl/common/log.h"
#include <cuda.h>

namespace ppl { namespace common {

RetCode InitCudaEnv(int device_id) {
    const char* errmsg = nullptr;
    auto cu_status = cuInit(0);
    if (cu_status != CUDA_SUCCESS) {
        cuGetErrorString(cu_status, &errmsg);
        LOG(ERROR) << "cuInit failed: " << errmsg;
        return RC_INTERNAL_ERROR;
    }

    CUcontext cu_context;
    cu_status = cuDevicePrimaryCtxRetain(&cu_context, device_id);
    if (cu_status != CUDA_SUCCESS) {
        cuGetErrorString(cu_status, &errmsg);
        LOG(ERROR) << "cuDevicePrimaryCtxRetain failed: " << errmsg;
        return RC_INTERNAL_ERROR;
    }

    cu_status = cuCtxSetCurrent(cu_context);
    if (cu_status != CUDA_SUCCESS) {
        cuGetErrorString(cu_status, &errmsg);
        LOG(ERROR) << "cuCtxSetCurrent failed: " << errmsg;
        return RC_INTERNAL_ERROR;
    }

    return RC_SUCCESS;
}

RetCode DestroyCudaEnv(int device_id) {
    auto cu_status = cuDevicePrimaryCtxRelease(device_id);
    if (cu_status != CUDA_SUCCESS) {
        const char* errmsg = nullptr;
        cuGetErrorString(cu_status, &errmsg);
        LOG(ERROR) << "cuDevicePrimaryCtxRelease failed: " << errmsg;
        return RC_INTERNAL_ERROR;
    }
    return RC_SUCCESS;
}

}}
