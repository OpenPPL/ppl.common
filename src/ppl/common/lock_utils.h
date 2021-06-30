#ifndef _ST_HPC_PPL_COMMON_LOCK_UTILS_H_
#define _ST_HPC_PPL_COMMON_LOCK_UTILS_H_

namespace ppl { namespace common {

class DummyLock final {
public:
    void ReadLock() {}
    void WriteLock() {}
    void Unlock() {}
};

template <typename T>
class ReadLockGuard final {
public:
    ReadLockGuard(T* lock) : lock_(lock) {
        lock->ReadLock();
    }
    ~ReadLockGuard() {
        lock_->Unlock();
    }

private:
    T* lock_;
};

template <typename T>
class WriteLockGuard final {
public:
    WriteLockGuard(T* lock) : lock_(lock) {
        lock->WriteLock();
    }
    ~WriteLockGuard() {
        lock_->Unlock();
    }

private:
    T* lock_;
};

}} // namespace ppl::common

#endif
