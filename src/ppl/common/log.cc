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

#include "ppl/common/log.h"
#include "ppl/common/stripfilename.h"
#include <memory>

#if defined(_WIN32) || defined(_WIN64)
    #include <cstddef>
    #include <windows.h>
    #if _MSC_VER < 1900
        #define snprintf _snprintf
    #endif
#else
    #include <sys/time.h>
#endif

using namespace std;

namespace ppl { namespace common {

static const char* g_log_level_str[] = {
    "DEBUG", "INFO", "WARNING", "ERROR", "FATAL",
};

LogMessage::LogMessage(uint32_t level, Logger* logger, const char* filename, uint32_t linenum)
    : level_(level), linenum_(linenum), logger_(logger), filename_(filename) {
    content_.reserve(4096);
}

static int FormatCurrentTime(char* time_buf) {
#if defined(_WIN32) || defined(_WIN64)
    SYSTEMTIME time;
    FILETIME fileTime;
    GetSystemTimeAsFileTime(&fileTime);
    FileTimeToSystemTime(&fileTime, &time);
    return sprintf(time_buf, "%04d-%02d-%02d %02d:%02d:%02d.%03d", time.wYear, time.wMonth, time.wDay, time.wHour + 8,
                   time.wMinute, time.wSecond, time.wMilliseconds);
#else
    time_t curtime = time(NULL);
    tm* ptm = localtime(&curtime);
    struct timeval tv;
    gettimeofday(&tv, NULL);
    int msec = tv.tv_usec / 1000;
    return sprintf(time_buf, "%04d-%02d-%02d %02d:%02d:%02d.%03d", ptm->tm_year + 1900, ptm->tm_mon + 1, ptm->tm_mday,
                   ptm->tm_hour, ptm->tm_min, ptm->tm_sec, msec);
#endif
}

LogMessage::~LogMessage() {
    char tmp_buf[128];
    auto tmp_buflen = FormatCurrentTime(tmp_buf);

    string log_prefix;
    log_prefix.reserve(1024);

    log_prefix.append("[");
    log_prefix.append(g_log_level_str[level_]);
    log_prefix.append("][");
    log_prefix.append(tmp_buf, tmp_buflen);
    log_prefix.append("]");

    log_prefix.append("[");
    log_prefix.append(stripfilename(filename_));
    tmp_buflen = snprintf(tmp_buf, 128, ":%u] ", linenum_);
    log_prefix.append(tmp_buf, tmp_buflen);

    logger_->Write(level_, log_prefix.data(), log_prefix.size(), content_.data(), content_.size());
}

#define DEF_READ_OPERATOR_FUNC(Type, fmt)            \
    LogMessage& LogMessage::operator<<(Type value) { \
        char buf[128];                               \
        auto len = snprintf(buf, 128, fmt, value);   \
        content_.append(buf, len);                   \
        return *this;                                \
    }

DEF_READ_OPERATOR_FUNC(void*, "%p");
DEF_READ_OPERATOR_FUNC(const void*, "%p");

/* -------------------------------------------------------------------------- */

static const char* g_color_begin_str[] = {
    "\033[0;34m", // blue -> DEBUG
    "\033[0;32m", // green -> INFO
    "\033[0;33m", // yellow -> WARNING
    "\033[0;31m", // red -> ERROR
    "\033[0;31m", // red -> FATAL
};

#define g_color_begin_str_len 7
#define g_color_end_str "\033[0m"
#define g_color_end_str_len 4

class StdioLogger final : public Logger {
public:
    StdioLogger(uint32_t level) : Logger(level) {}
    void Write(uint32_t level, const char* prefix, uint64_t prefix_len, const char* content,
               uint64_t content_len) override {
        auto fp = ((level <= LOG_LEVEL_INFO) ? stdout : stderr);
#ifdef PPLCOMMON_STDIO_LOG_COLOR
        fwrite(g_color_begin_str[level], 1, g_color_begin_str_len, fp);
#endif
        fwrite(prefix, 1, prefix_len, fp);
#ifdef PPLCOMMON_STDIO_LOG_COLOR
        fwrite(g_color_end_str, 1, g_color_end_str_len, fp);
#endif
        fwrite(content, 1, content_len, fp);
        fwrite("\n", 1, 1, fp);
        fflush(fp);
    }
};

/* -------------------------------------------------------------------------- */

#ifdef NDEBUG
static shared_ptr<Logger> g_logger = make_shared<StdioLogger>(LOG_LEVEL_INFO);
#else
static shared_ptr<Logger> g_logger = make_shared<StdioLogger>(LOG_LEVEL_DEBUG);
#endif

static void DummyDeleter(Logger*) {}

void SetCurrentLogger(Logger* logger) {
    g_logger = shared_ptr<Logger>(logger, DummyDeleter);
}

Logger* GetCurrentLogger() {
    return g_logger.get();
}

}} // namespace ppl::common
