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

#include "ppl/common/sys.h"
#include "ppl/common/log.h"
#include "ppl/common/x86/sysinfo.h"

int main () {
    ppl::common::CpuInfo info_id, info_run;
    ppl::common::GetCPUInfoByCPUID(&info_id);
    ppl::common::GetCPUInfoByRun(&info_run);

    LOG(INFO) << "Info From CPUID:";
    LOG(INFO) << "VNNI  : " << (bool)(info_id.isa & ppl::common::ISA_X86_AVX512VNNI);
    LOG(INFO) << "AVX512: " << (bool)(info_id.isa & ppl::common::ISA_X86_AVX512);
    LOG(INFO) << "AVX2  : " << (bool)(info_id.isa & ppl::common::ISA_X86_AVX2);
    LOG(INFO) << "FMA3  : " << (bool)(info_id.isa & ppl::common::ISA_X86_FMA);
    LOG(INFO) << "AVX   : " << (bool)(info_id.isa & ppl::common::ISA_X86_AVX);
    LOG(INFO) << "SSE   : " << (bool)(info_id.isa & ppl::common::ISA_X86_SSE);
    LOG(INFO) << "L1 D-Cahce: " << info_id.l1_cache_size;
    LOG(INFO) << "L2 Cahce: " << info_id.l2_cache_size;
    LOG(INFO) << "L3 Cahce: " << info_id.l3_cache_size;

    LOG(INFO) << "Info From RUN:";
    LOG(INFO) << "VNNI  : " << (bool)(info_run.isa & ppl::common::ISA_X86_AVX512VNNI);
    LOG(INFO) << "AVX512: " << (bool)(info_run.isa & ppl::common::ISA_X86_AVX512);
    LOG(INFO) << "AVX2  : " << (bool)(info_run.isa & ppl::common::ISA_X86_AVX2);
    LOG(INFO) << "FMA3  : " << (bool)(info_run.isa & ppl::common::ISA_X86_FMA);
    LOG(INFO) << "AVX   : " << (bool)(info_run.isa & ppl::common::ISA_X86_AVX);
    LOG(INFO) << "SSE   : " << (bool)(info_run.isa & ppl::common::ISA_X86_SSE);
    LOG(INFO) << "L1 D-Cahce: " << info_run.l1_cache_size;
    LOG(INFO) << "L2 Cahce: " << info_run.l2_cache_size;
    LOG(INFO) << "L3 Cahce: " << info_run.l3_cache_size;

    return 0;
}
