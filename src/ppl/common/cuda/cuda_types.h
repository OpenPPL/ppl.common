#ifndef _ST_HPC_PPL_COMMON_CUDA_CUDA_TYPES_H_
#define _ST_HPC_PPL_COMMON_CUDA_CUDA_TYPES_H_

#include <stdint.h>

namespace ppl { namespace common { namespace cuda {

static inline uint32_t GetDataFormatChannelAlignment(dataformat_t dt) {
    static const uint32_t data_format_alignment[] = {
        0, // UNKNOWN
        1, // NDARRAY
        8, // NHWC
        2, // N2CX
        4, // N4CX
        8, // N8CX
        16, // N16CX
        32, // N32CX
    };
    return data_format_alignment[dt];
}

}}} // namespace ppl::common::cuda

#endif
