#ifndef _ST_HPC_PPL_COMMON_FILE_MAPPING_H_
#define _ST_HPC_PPL_COMMON_FILE_MAPPING_H_

#include "ppl/common/retcode.h"

#ifdef _MSC_VER
#define NO_MINMAX
#define WIN32_LEAN_AND_MEAN
#define NOGDI
#include <windows.h>
#endif

namespace ppl { namespace common {

class FileMapping final {
private:
    static const uint32_t MAX_MSG_BUF_SIZE = 1024;

public:
    FileMapping();
    ~FileMapping();

    ppl::common::RetCode Init(const char* filename);
    const char* Data() const {
        return (const char*)buffer_;
    }
    uint64_t Size() const {
        return size_;
    }
    const char* GetErrorMessage() const {
        return error_message_;
    }

private:
#ifdef _MSC_VER
    HANDLE h_file_;
    HANDLE h_map_file_;
#else
    int file_fd_;
#endif
    void* buffer_;
    uint64_t size_;
    char error_message_[MAX_MSG_BUF_SIZE];
};

}} // namespace ppl::common

#endif
