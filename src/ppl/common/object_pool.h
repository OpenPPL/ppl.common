#ifndef _ST_HPC_PPL_COMMON_OBJECT_POOL_H_
#define _ST_HPC_PPL_COMMON_OBJECT_POOL_H_

#include "ppl/common/generic_cpu_allocator.h"
#include "ppl/common/lock_utils.h"
#include <vector>
#include <utility> // std::forward

namespace ppl { namespace common {

template <typename T, typename LockType = DummyLock>
class ObjectPool final {
public:
    ObjectPool() {
        Reserve();
    }
    ~ObjectPool() {
        Clear();
    }

    void Clear() {
        WriteLockGuard<LockType> __guard__(&lock_);

        for (auto x = slabs_.begin(); x != slabs_.end(); ++x) {
            allocator_.Free(*x);
        }
        slabs_.clear();
        free_objects_.clear();
    }

    template <typename... Args>
    T* Alloc(Args&&... args) {
        WriteLockGuard<LockType> __guard__(&lock_);

        if (free_objects_.empty()) {
            Reserve();
            if (free_objects_.empty()) {
                return nullptr;
            }
        }

        auto o = free_objects_.back();
        free_objects_.pop_back();

        return new (o) T(std::forward<Args>(args)...);
    }

    void Free(T* obj) {
        if (obj) {
            WriteLockGuard<LockType> __guard__(&lock_);
            obj->~T();
            free_objects_.push_back(obj);
        }
    }

private:
    void Reserve() {
        auto slab = static_cast<T*>(allocator_.Alloc(sizeof(T) * PREALLOC_NUM));
        if (!slab) {
            return;
        }

        slabs_.push_back(slab);

        free_objects_.reserve(free_objects_.size() + PREALLOC_NUM);
        for (uint32_t i = 0; i < PREALLOC_NUM; ++i) {
            free_objects_.push_back(slab);
            ++slab;
        }
    }

private:
    static constexpr uint32_t PREALLOC_NUM = 32;

private:
    LockType lock_;
    std::vector<T*> free_objects_;
    std::vector<void*> slabs_;
    GenericCpuAllocator allocator_;
};

}} // namespace ppl::common

#endif
