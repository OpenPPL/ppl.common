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

#ifndef _ST_HPC_PPL_COMMON_WINDOWS_PTHREAD_H_
#define _ST_HPC_PPL_COMMON_WINDOWS_PTHREAD_H_

// XXX NOTE: partial pthread support for windows. search param name "unsupported".

#define NOGDI
#include <cstddef>
#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */

typedef HANDLE pthread_t;

int pthread_create(pthread_t*, void* unsupported, void* (*f)(void*), void* arg);
int pthread_join(pthread_t, void* unsupported);
int pthread_detach(pthread_t);
int pthread_cancel(pthread_t);
pthread_t pthread_self();

/* -------------------------------------------------------------------------- */

typedef CRITICAL_SECTION pthread_mutex_t;

int pthread_mutex_init(pthread_mutex_t*, void* unsupported);
int pthread_mutex_lock(pthread_mutex_t*);
int pthread_mutex_unlock(pthread_mutex_t*);
int pthread_mutex_destroy(pthread_mutex_t*);

/* -------------------------------------------------------------------------- */

typedef CONDITION_VARIABLE pthread_cond_t;

int pthread_cond_init(pthread_cond_t*, void* unsupported);
int pthread_cond_signal(pthread_cond_t*);
int pthread_cond_broadcast(pthread_cond_t*);
int pthread_cond_wait(pthread_cond_t*, pthread_mutex_t* mutex);
int pthread_cond_destroy(pthread_cond_t*);

/* -------------------------------------------------------------------------- */

#define PTHREAD_ONCE_INIT 0

typedef LONG pthread_once_t;

int pthread_once(pthread_once_t*, void (*f)(void));

#ifdef __cplusplus
}
#endif

#endif
