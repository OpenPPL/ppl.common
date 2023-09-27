// Licensed to the Apache Software Foundation (ASF) under one
// or more contributor license agreements.  See the NOTICE file
// distributed with this work for additional information
// regarding copyright ownership.  The ASF licenses this file
// to you under the Apache License, Version 2.0 (the
// "License"); you may not use this file except in compliance
// with the License.  You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing,
// software distributed under the License is distributed on an
// "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied.  See the License for the
// specific language governing permissions and limitations
// under the License.

#include "ppl/common/threadpool.h"
#include "ppl/common/log.h"
#include <cstdlib>
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
    ThreadTaskQueue* queue;
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

RetCode ThreadPool::AddTask(const shared_ptr<ThreadTask>& task, uint32_t queue_idx) {
    if (!task) {
        return RC_INVALID_VALUE;
    }

    queues_[queue_idx].Push(task);
    return RC_SUCCESS;
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

RetCode ThreadPool::Init(uint32_t thread_num, bool share_task_queue) {
    if (thread_num == 0) {
        if (cpu_core_num_ > 1) {
            thread_num = cpu_core_num_ - 1;
        } else {
            thread_num = 1;
        }
    }

    if (share_task_queue) {
        queues_ = (ThreadTaskQueue*)malloc(sizeof(ThreadTaskQueue));
        if (!queues_) {
            return RC_OUT_OF_MEMORY;
        }
        new (queues_) ThreadTaskQueue();
        queue_num_ = 1;
    } else {
        queues_ = (ThreadTaskQueue*)malloc(thread_num * sizeof(ThreadTaskQueue));
        if (!queues_) {
            return RC_OUT_OF_MEMORY;
        }
        for (uint32_t i = 0; i < thread_num; ++i) {
            new (queues_ + i) ThreadTaskQueue();
        }
        queue_num_ = thread_num;
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
        if (share_task_queue) {
            args[i].queue = queues_;
        } else {
            args[i].queue = queues_ + i;
        }
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

void ThreadPool::Destroy() {
    if (threads_.empty()) {
        return;
    }

    // push null task to kill a thread
    shared_ptr<ThreadTask> dummy_task;
    if (queue_num_ == threads_.size()) {
        for (uint32_t i = 0; i < threads_.size(); ++i) {
            queues_[i].Push(dummy_task);
        }
    } else {
        for (uint32_t i = 0; i < threads_.size(); ++i) {
            queues_[0].Push(dummy_task);
        }
    }

    for (uint32_t i = 0; i < threads_.size(); ++i) {
        pthread_join(threads_[i].pid, nullptr);
    }
    threads_.clear();

    if (queues_) {
        for (uint32_t i = 0; i < queue_num_; ++i) {
            queues_[i].~ThreadTaskQueue();
        }
        free(queues_);
    }
    queues_ = nullptr;
}

/* ------------------------------------------------------------------------- */

void StaticThreadPool::Destroy() {
    func_ = nullptr;
    barrier_.Wait();
    for (auto t = threads_.begin(); t != threads_.end(); ++t) {
        pthread_join(t->pid, nullptr);
    }
    threads_.clear();
}

RetCode StaticThreadPool::Init(uint32_t thread_num) {
    if (thread_num == 0) {
#ifdef _MSC_VER
        SYSTEM_INFO info;
        GetSystemInfo(&info);
        auto cpu_core_num = info.dwNumberOfProcessors;
#else
        auto cpu_core_num = sysconf(_SC_NPROCESSORS_ONLN);
#endif
        if (cpu_core_num > 1) {
            thread_num = cpu_core_num - 1;
        } else {
            thread_num = 1;
        }
    }

    barrier_.Reset(thread_num + 1);
    threads_.resize(thread_num);
    for (uint32_t i = 0; i < thread_num; ++i) {
        threads_[i].thread_idx = i;
        threads_[i].pool = this;

        if (pthread_create(&threads_[i].pid, nullptr, ThreadWorker, &threads_[i]) != 0) {
            barrier_.Reset(i + 1);
            threads_.resize(i);
            return RC_OTHER_ERROR;
        }
    }

    return RC_SUCCESS;
}

void* StaticThreadPool::ThreadWorker(void* arg) {
    auto info = (ThreadInfo*)arg;
    auto pool = info->pool;

    while (true) {
        pool->barrier_.Wait();
        if (!pool->func_) {
            break;
        }
        pool->func_(pool->threads_.size(), info->thread_idx);
        pool->barrier_.Wait();
    }

    return nullptr;
}

void StaticThreadPool::Run(const function<void(uint32_t nr_threads, uint32_t thread_idx)>& f) {
    RunAsync(f);
    Wait();
}

void StaticThreadPool::RunAsync(const function<void(uint32_t nr_threads, uint32_t thread_idx)>& f) {
    func_ = f;
    Wait(); // notify start
}

void StaticThreadPool::Wait() {
    barrier_.Wait(); // wait for end
}

}}
