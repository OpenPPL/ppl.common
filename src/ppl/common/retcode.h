#ifndef _ST_HPC_PPL_COMMON_RETCODE_H_
#define _ST_HPC_PPL_COMMON_RETCODE_H_

#include <stdint.h>

namespace ppl { namespace common {

typedef uint32_t RetCode;

enum {
    RC_SUCCESS = 0,
    RC_OTHER_ERROR,
    RC_UNSUPPORTED,
    RC_OUT_OF_MEMORY,
    RC_INVALID_VALUE,
    RC_EXISTS,
    RC_NOT_FOUND,
    RC_PERMISSION_DENIED,
};

static inline const char* GetRetCodeStr(RetCode rc) {
    static const char* code_str[] = {
        "success",       "other error", "unsupported", "out of memory",
        "invalid value", "exists",      "not found",   "perimission denied",
    };
    return code_str[rc];
}

}} // namespace ppl::common

#endif
