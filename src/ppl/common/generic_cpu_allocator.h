#ifndef _ST_HPC_PPL_COMMON_GENERIC_CPU_ALLOCATOR_H_
#define _ST_HPC_PPL_COMMON_GENERIC_CPU_ALLOCATOR_H_

#include "ppl/common/allocator.h"
#include "ppl/common/sys.h"
#include <new>
#include <utility> // std::forward

namespace ppl { namespace common {

class GenericCpuAllocator : public Allocator {
public:
    GenericCpuAllocator(uint64_t alignment = 64) : Allocator(alignment) {}
    virtual ~GenericCpuAllocator() {}

    template <typename T, typename... Args>
    T* TypedAlloc(Args&&... args) {
        auto obj = (T*)Alloc(sizeof(T));
        if (obj) {
            new (obj) T(std::forward<Args>(args)...);
        }
        return obj;
    }

    template <typename T>
    void TypedFree(T* obj) {
        if (obj) {
            obj->~T();
            Free(obj);
        }
    }

    void* Alloc(uint64_t size) override final {
        return ppl::common::AlignedAlloc(size, alignment_);
    }

    void Free(void* ptr) override final {
        ppl::common::AlignedFree(ptr);
    }
};

}} // namespace ppl::common

#endif
