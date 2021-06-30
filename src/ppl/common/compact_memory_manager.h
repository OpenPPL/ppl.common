#ifndef _ST_HPC_PPL_COMMON_COMPACT_MEMORY_MANAGER_H_
#define _ST_HPC_PPL_COMMON_COMPACT_MEMORY_MANAGER_H_

#include "ppl/common/allocator.h"
#include <vector>
#include <map>
#include <set>

namespace ppl { namespace common {

class CompactMemoryManager final {
public:
    /** @param block_bytes MUST be power of 2 */
    CompactMemoryManager(Allocator* ar, uint64_t block_bytes = 1048576);
    ~CompactMemoryManager();
    void* Alloc(uint64_t bytes);
    void Free(void* addr, uint64_t bytes);
    void Clear();
    uint64_t GetAllocatedBytes() const {
        return allocated_bytes_;
    }
    Allocator* GetAllocator() const {
        return allocator_;
    }

private:
    const uint64_t block_bytes_;
    uint64_t allocated_bytes_ = 0;
    Allocator* allocator_;
    std::map<void*, uint64_t> addr2bytes_;
    std::map<uint64_t, std::set<void*>> bytes2addr_;
    std::vector<void*> chunks_;
};

}} // namespace ppl::common

#endif
