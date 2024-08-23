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

#include "nccl_utils.h"

namespace ppl { namespace common {

#ifdef PPLCOMMON_ENABLE_NCCL
RetCode InitNccl(uint32_t tensor_parallel_size, std::vector<ncclComm_t>* nccl_comm_list) {
    nccl_comm_list->resize(tensor_parallel_size);
    std::vector<int> dev_list(tensor_parallel_size);
    for (size_t i = 0; i < tensor_parallel_size; ++i) {
        dev_list[i] = i;
    }
    NCCL_CHECK(ncclCommInitAll(nccl_comm_list->data(), tensor_parallel_size, dev_list.data()), "ncclCommInitAll");
    return RC_SUCCESS;
}
#endif

}} // namespace ppl::common