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

#ifndef _ST_HPC_PPL_COMMON_X86_SYSINFO_H_
#define _ST_HPC_PPL_COMMON_X86_SYSINFO_H_

namespace ppl { namespace common {

enum {
    ISA_X86_SSE = 0x1,
    ISA_X86_SSE2 = 0x2,
    ISA_X86_SSE3 = 0x4,
    ISA_X86_SSSE3 = 0x8,
    ISA_X86_SSE41 = 0x10,
    ISA_X86_SSE42 = 0x20,
    ISA_X86_AVX = 0x40,
    ISA_X86_AVX2 = 0x80,
    ISA_X86_FMA = 0x100,
    ISA_X86_F16C = 0x200,
    ISA_X86_AVX512 = 0x1000,
};

}}

#endif
