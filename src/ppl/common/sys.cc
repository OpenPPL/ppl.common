#include "ppl/common/sys.h"
#include <string.h>
#include <stdlib.h>
#ifdef _WIN32
#define WIN32_LEAN_MEAN
#define NOGDI // remove ERROR def
#include <malloc.h>
#include <Windows.h>
#endif

namespace ppl { namespace common {

static inline void* AlignedAlloc_impl(uint64_t size, uint32_t alignment) {
#if defined(_WIN32)
    return _aligned_malloc(size, alignment);
#elif defined(__ANDROID__)
    return memalign(alignment, size);
#elif defined(__linux__) || defined(__APPLE__) || defined(__QNX__)
    void* p = nullptr;
    int q = posix_memalign(&p, alignment, size);
    if (q == 0) {
        return p;
    }
    return nullptr;
#else
#error "Unknown platform."
#endif // _WIN32
}

static inline void AlignedFree_impl(void* p) {
#if defined(_WIN32)
    _aligned_free(p);
#else
    free(p);
#endif
}

void* AlignedAlloc(uint64_t size, uint32_t alignment) {
    return AlignedAlloc_impl(size, alignment);
}
void AlignedFree(void* p) {
    AlignedFree_impl(p);
}

}} // namespace ppl::common
