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

#ifndef _ST_HPC_PPL_COMMON_LOG_H_
#define _ST_HPC_PPL_COMMON_LOG_H_

#include "ppl/common/retcode.h"
#include <string>

namespace ppl { namespace common {

enum {
    LOG_LEVEL_DEBUG = 0,
    LOG_LEVEL_INFO = 1,
    LOG_LEVEL_WARNING = 2,
    LOG_LEVEL_ERROR = 3,
    LOG_LEVEL_FATAL = 4,
    LOG_LEVEL_MAX,
};

class Logger {
public:
    Logger(uint32_t level) : level_(level) {}
    virtual ~Logger() {}

    void SetLogLevel(uint32_t l) {
        level_ = l;
    }
    uint32_t GetLogLevel() const {
        return level_;
    }

    virtual void Write(uint32_t level, const char* log_prefix, uint64_t prefix_len, const char* log_content,
                       uint64_t content_len) = 0;

private:
    uint32_t level_;
};

class LogMessage final {
public:
    LogMessage(uint32_t, Logger*, const char* filename, int linenum);
    ~LogMessage();

    LogMessage& operator<<(float f);
    LogMessage& operator<<(double d);

    LogMessage& operator<<(int8_t i8);
    LogMessage& operator<<(uint8_t u8);
    LogMessage& operator<<(int16_t i16);
    LogMessage& operator<<(uint16_t u16);
    LogMessage& operator<<(int32_t i32);
    LogMessage& operator<<(uint32_t u32);
    LogMessage& operator<<(int64_t i64);
    LogMessage& operator<<(uint64_t u64);

#if defined(__APPLE__) && (defined(__GNUC__) || defined(__xlC__) || defined(__xlc__))
    LogMessage& operator<<(size_t s);
#elif !defined(_WIN32) && !defined(_WIN64)
    LogMessage& operator<<(long long ll);
    LogMessage& operator<<(unsigned long long ull);
#endif

    LogMessage& operator<<(const char*);
    LogMessage& operator<<(const std::string&);

    LogMessage& operator<<(const void* p);

private:
    uint32_t level_;
    Logger* logger_;
    const char* filename_;
    int linenum_;
    std::string content_;
};

class DummyOperatorPlaceHolder final {
public:
    void operator&(const LogMessage&) const {}
};

void SetCurrentLogger(Logger*);
Logger* GetCurrentLogger();

#define LOG(___level___)                                                                                             \
    (ppl::common::LOG_LEVEL_##___level___ < ppl::common::GetCurrentLogger()->GetLogLevel())                          \
        ? (void)0                                                                                                    \
        : ppl::common::DummyOperatorPlaceHolder() &                                                                  \
            ppl::common::LogMessage(ppl::common::LOG_LEVEL_##___level___, ppl::common::GetCurrentLogger(), __FILE__, \
                                    __LINE__)

}} // namespace ppl::common

#endif
