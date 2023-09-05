#include "ppl/common/threadpool.h"
using namespace std;

#ifndef _MSC_VER
#include <sched.h> // for CPU_*
#include <unistd.h>
#ifdef HAVE_SYS_GETTID
#include <sys/syscall.h> // for syscall
#endif // HAVE_SYS_GETTID
#endif

namespace ppl { namespace common {

JoinableThreadTask::JoinableThreadTask() {
    pthread_mutex_init(&mutex_, nullptr);
    pthread_cond_init(&cond_, nullptr);
}

JoinableThreadTask::~JoinableThreadTask() {
    pthread_cond_destroy(&cond_);
    pthread_mutex_destroy(&mutex_);
}

shared_ptr<ThreadTask> JoinableThreadTask::Run() {
    shared_ptr<ThreadTask> next_task;
    if (!IsFinished()) {
        pthread_mutex_lock(&mutex_);
        next_task = Process();
        pthread_cond_signal(&cond_);
        pthread_mutex_unlock(&mutex_);
    }
    return next_task;
}

void JoinableThreadTask::Join() {
    pthread_mutex_lock(&mutex_);
    while (!IsFinished()) {
        pthread_cond_wait(&cond_, &mutex_);
    }
    pthread_mutex_unlock(&mutex_);
}

/* -------------------------------------------------------------------------- */

struct ThreadArg final {
    uint32_t expected_count_for_init;
    uint32_t* count_for_init;
    pthread_mutex_t* mutex_for_init;
    pthread_cond_t* cond_for_init;
    ThreadPool::ThreadInfo* info;
    MessageQueue<shared_ptr<ThreadTask>>* queue;
};

void* ThreadPool::ThreadWorker(void* thread_arg) {
    auto arg = (ThreadArg*)thread_arg;
    auto q = arg->queue;

    arg->info->pid = pthread_self();
#ifdef HAVE_SYS_GETTID
#ifndef _MSC_VER
    arg->info->tid = syscall(SYS_gettid);
#endif
#endif

    pthread_mutex_lock(arg->mutex_for_init);
    ++(*(arg->count_for_init));
    if (*(arg->count_for_init) == arg->expected_count_for_init) {
        pthread_cond_signal(arg->cond_for_init);
    }
    pthread_mutex_unlock(arg->mutex_for_init);

    while (true) {
        auto task = q->Pop();
        if (!task) {
            break;
        }

        do {
            task = task->Run();
        } while (task);
    }

    return nullptr;
}

RetCode ThreadPool::AddTask(const shared_ptr<ThreadTask>& task) {
    if (task) {
        queue_.Push(task);
        return RC_SUCCESS;
    }
    return RC_INVALID_VALUE;
}

#ifdef _MSC_VER
RetCode ThreadPool::SetAffinity(uint32_t thread_id, const uint32_t* core_list,
                                uint32_t core_num) {
    if (thread_id >= threads_.size()) {
        return RC_INVALID_VALUE;
    }

    size_t cpu_set = 0;
    for (uint32_t i = 0; i < core_num; ++i) {
        const uint32_t core_id = core_list[i];
        if (core_id >= cpu_core_num_ || core_id >= sizeof(size_t)) {
            return RC_INVALID_VALUE;
        }

        cpu_set |= (1ull << core_id);
    }

    if (SetThreadAffinityMask(GetCurrentThread(), cpu_set) != 0) {
        return RC_SUCCESS;
    }

    return RC_OTHER_ERROR;
}
#else
RetCode ThreadPool::SetAffinity(uint32_t thread_id, const uint32_t* core_list,
                                uint32_t core_num) {
#if !defined(__APPLE__) && !defined(__QNX__)
    if (thread_id >= threads_.size()) {
        return RC_INVALID_VALUE;
    }

    cpu_set_t* cpu_set = CPU_ALLOC(cpu_core_num_);
    if (!cpu_set) {
        return RC_OUT_OF_MEMORY;
    }

    const size_t cpu_setsize = CPU_ALLOC_SIZE(cpu_core_num_);
    CPU_ZERO_S(cpu_setsize, cpu_set);

    bool ok = false;
    for (uint32_t i = 0; i < core_num; ++i) {
        const uint32_t core_id = core_list[i];
        if (core_id >= cpu_core_num_) {
            goto end;
        }
        CPU_SET_S(core_id, cpu_setsize, cpu_set);
    }

    // Android doesn't support pthread_setaffinity_np()
    ok = (sched_setaffinity(threads_[thread_id].tid, cpu_setsize, cpu_set) == 0);
end:
    CPU_FREE(cpu_set);
    return (ok ? RC_SUCCESS : RC_OTHER_ERROR);
#else
    return RC_UNSUPPORTED;
#endif
}
#endif

ThreadPool::ThreadPool() {
#ifdef _MSC_VER
    SYSTEM_INFO info;
    GetSystemInfo(&info);
    cpu_core_num_ = info.dwNumberOfProcessors;
#else
    cpu_core_num_ = sysconf(_SC_NPROCESSORS_ONLN);
#endif
}

RetCode ThreadPool::Init(uint32_t thread_num) {
    if (thread_num == 0) {
        if (cpu_core_num_ > 1) {
            thread_num = cpu_core_num_ - 1;
        } else {
            thread_num = 1;
        }
    }

    threads_.resize(thread_num);

    uint32_t count_for_init = 0;
    pthread_mutex_t mutex_for_init;
    pthread_cond_t cond_for_init;

    pthread_mutex_init(&mutex_for_init, nullptr);
    pthread_cond_init(&cond_for_init, nullptr);

    vector<ThreadArg> args(thread_num);
    for (uint32_t i = 0; i < thread_num; ++i) {
        args[i].expected_count_for_init = thread_num;
        args[i].count_for_init = &count_for_init;
        args[i].mutex_for_init = &mutex_for_init;
        args[i].cond_for_init = &cond_for_init;
        args[i].info = &threads_[i];
        args[i].queue = &queue_;
    }
    for (uint32_t i = 0; i < thread_num; ++i) {
        pthread_t pid;
        if (pthread_create(&pid, nullptr, ThreadWorker, &args[i]) != 0) {
            return RC_OTHER_ERROR;
        }
    }

    // waiting for thread(s) to finish init
    pthread_mutex_lock(&mutex_for_init);
    pthread_cond_wait(&cond_for_init, &mutex_for_init);
    pthread_mutex_unlock(&mutex_for_init);

    pthread_cond_destroy(&cond_for_init);
    pthread_mutex_destroy(&mutex_for_init);

    return RC_SUCCESS;
}

ThreadPool::~ThreadPool() {
    shared_ptr<ThreadTask> dummy_task;
    for (uint32_t i = 0; i < threads_.size(); ++i) {
        // push null task to kill a thread
        queue_.Push(dummy_task);
    }
    for (uint32_t i = 0; i < threads_.size(); ++i) {
        pthread_join(threads_[i].pid, nullptr);
    }
}

}}
