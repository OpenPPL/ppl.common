#ifndef _ST_HPC_PPL_COMMON_ALLOCATOR_H_
#define _ST_HPC_PPL_COMMON_ALLOCATOR_H_

#include <stdint.h>

namespace ppl { namespace common {

class Allocator {
public:
    Allocator(uint64_t alignment) : alignment_(alignment) {}
    virtual ~Allocator() {}
    virtual void* Alloc(uint64_t size) = 0;
    virtual void Free(void* ptr) = 0;
    uint64_t GetAlignment() const { return alignment_; }
protected:
    const uint64_t alignment_;
};

}} // namespace ppl::common

#endif
