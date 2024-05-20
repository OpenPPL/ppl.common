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

#include "futex_wrapper.h"

namespace ppl { namespace common {

#ifdef _MSC_VER

#include <windows.h>

void FutexWait(uint32_t* addr, uint32_t value) {
    WaitOnAddress(addr, &value, sizeof(uint32_t), INFINITE);
}

void FutexWakeOne(uint32_t* addr) {
    WakeByAddressSingle(addr);
}

void FutexWakeAll(uint32_t* addr) {
    WakeByAddressAll(addr);
}

#else

#include <linux/futex.h>
#include <sys/syscall.h> // SYS_futex
#include <unistd.h> // syscall()
#include <climits>

// refer to http://locklessinc.com/articles/futex_cheat_sheet/

void FutexWait(uint32_t* addr, uint32_t value) {
	syscall(SYS_futex, addr, FUTEX_WAIT_PRIVATE, value, nullptr, nullptr, 0);
}

void FutexWakeOne(uint32_t* addr) {
	syscall(SYS_futex, addr, FUTEX_WAKE_PRIVATE, 1, nullptr, nullptr, 0);
}

void FutexWakeAll(uint32_t* addr) {
	syscall(SYS_futex, addr, FUTEX_WAKE_PRIVATE, INT_MAX, nullptr, nullptr, 0);
}

#endif

}}
