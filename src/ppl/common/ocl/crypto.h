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

#ifndef _ST_HPC_PPL_COMMON_OCL_CRYPTO_H_
#define _ST_HPC_PPL_COMMON_OCL_CRYPTO_H_

#include <stdint.h>
#include <vector>
namespace ppl { namespace common { namespace ocl {

/**************************************************************************
 * AES declarations
 **************************************************************************/

#define AES_MAXROUNDS 14
#define AES_BLOCKSIZE 16
#define AES_IV_SIZE 16

#define ROUNDUP(n, align) (((n) + (align)-1) & (~((align)-1)))

using namespace std;

typedef struct aes_key_st {
    uint16_t rounds;
    uint16_t key_size;
    uint32_t ks[(AES_MAXROUNDS + 1) * 8];
    uint8_t iv[AES_IV_SIZE];
} AES_CTX;

typedef enum { AES_MODE_128, AES_MODE_256 } AES_MODE;

void AES_set_key(AES_CTX* ctx, const uint8_t* key, const uint8_t* iv, AES_MODE mode);
void AES_cbc_decrypt(AES_CTX* ks, const uint8_t* in, uint8_t* out, int length);
void AES_convert_key(AES_CTX* ctx);

bool DecryptContent(const char* src, uint32_t size, vector<char>* dst);

}}} // namespace ppl::common::ocl

#endif
