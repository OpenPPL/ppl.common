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

#ifndef _ST_HPC_PPL_COMMON_OCL_BINARY_H_
#define _ST_HPC_PPL_COMMON_OCL_BINARY_H_

#include <stddef.h>
#include <stdint.h>

namespace ppl { namespace common { namespace ocl {

    /*********Chunk
     * project name: nn/cv/math
     * name0: project_name + source file name/function name
     * name1: IoInfo 
     * kernel-name = name0 + IoInfo
     * source-name = name0;
     */

    /********Chunk Group
     * device info:
     */

    struct Chunk {
        char name0[128];
        char name1[512];
        uint32_t offset;     // offset: start from the ChunkGroup
        uint32_t size;       // size: data size
    };
    struct ChunkGroup {
        char ns[64];
        char device[128];
        char sha256[32];
        uint32_t flags;      // Encrypted, Compressed
        uint32_t size;       // sizeof(ChunkGroup) + sizeof(Chunk[numberChunks]) + data
        uint32_t count;
    };

#define ChunkGroup_Encrypted 0x01
#define ChunkGroup_Compressed 0x02
#define ChunkGroup_No_Copy 0x04

    /*!
     * @brief InsertKernelBinaryGroup
     * buffer structure:
     * KernelBinGroup: header
     * KernelBin: kernelHeaders[]
     * data[...]
     * @param ns
     * @param buffer
     * @param size
     * @param flags ChunkGroup_No_copy
     **********************************/
    int InsertKernelBinaryGroup(const uint8_t* buffer, size_t size, int flags);
    /*!
     * @brief ReleaseAllKernelBinaryGroup
     * only ppl developer can get kernel source, the upper user can't get them
     * thus we only need to supply the api of ReleaseAllKernelBinaryGroup for upper users.
     * there is no need to supply the api of ReleaseAllKernelSourceGroup
     *************************************************/
    int ReleaseAllKernelBinaryGroup();
    /*!
     * @brief LockKernelBinaryDB
     * 1. LockBinaryDB
     * 2. FindBinary and use binary
     * 3. UnlockBinaryDB
     *************************************************/
    void LockKernelBinaryDB();
    void UnlockKernelBinaryDB();
    int FindKernelBinary(const char* ns, const char* name0, const char* name1, const uint8_t** buffer, size_t* size);

    //------------------------------------------------//
    int InsertKernelSourceGroup(const uint8_t* buffer, size_t size, int flags);
    void LockKernelSourceDB();
    int KernelBinarySize();
    void UnlockKernelSourceDB();
    int FindKernelSource(const char* ns, const char* name0, const char* name1, const uint8_t** buffer, size_t* size);

    //------------------------------------------------//
    void InsertBinaryChunk(const char* ns, uint8_t* data, size_t size);
    void SaveKernelBinToFile(const char* dir = nullptr);

}}} // namespace ppl::common::ocl

#endif
