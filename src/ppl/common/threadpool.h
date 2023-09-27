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

#ifndef _ST_HPC_PPL_COMMON_THREADPOOL_H_
#define _ST_HPC_PPL_COMMON_THREADPOOL_H_

#include "ppl/common/retcode.h"
#include "ppl/common/message_queue.h"
#include "ppl/common/barrier.h"
#include <vector>
#include <memory>
#include <functional>

namespace ppl { namespace common {

class ThreadTask {
public:
    virtual ~ThreadTask() {}
    /**
       returns a task that will be executed right after Run() returns,
       or returns nullptr so that scheduler will pick up a task from task queue.
     */
    virtual std::shared_ptr<ThreadTask> Run() = 0;
};

class JoinableThreadTask : public ThreadTask {
public:
    std::shared_ptr<ThreadTask> Run() override final;
    void Join();

protected:
    JoinableThreadTask();
    virtual ~JoinableThreadTask();

    virtual bool IsFinished() const = 0;
    virtual std::shared_ptr<ThreadTask> Process() = 0;

private:
    pthread_mutex_t mutex_;
    pthread_cond_t cond_;

private:
    JoinableThreadTask(const JoinableThreadTask&) = delete;
    JoinableThreadTask(JoinableThreadTask&&) = delete;
    void operator=(const JoinableThreadTask&) = delete;
    void operator=(JoinableThreadTask&&) = delete;
};

typedef MessageQueue<std::shared_ptr<ThreadTask>> ThreadTaskQueue;

class ThreadPool final {
public:
    struct ThreadInfo final {
        pthread_t pid; // pthread_self()
#ifndef _MSC_VER
        pid_t tid; // gettid()
#endif
    };

public:
    ThreadPool();
    ~ThreadPool() {
        Destroy();
    }

    /**
       only queue 0 is available if `share_task_queue` is true,
       othrewise each thread has its own task queue.
    */
    ppl::common::RetCode Init(uint32_t thread_num = 0, bool share_task_queue = true);
    void Destroy();

    uint32_t GetThreadNum() const { return threads_.size(); }
    ppl::common::RetCode AddTask(const std::shared_ptr<ThreadTask>&, uint32_t queue_idx = 0);

    /**
       0 <= thread_id < ThreadNum()
       0 <= core_list[i] < sysconf(_SC_NPROCESSORS_ONLN)
     */
    ppl::common::RetCode SetAffinity(uint32_t thread_id, const uint32_t* core_list, uint32_t core_num);

private:
    static void* ThreadWorker(void*);

    std::vector<ThreadInfo> threads_;
    ThreadTaskQueue* queues_ = nullptr;
    uint32_t queue_num_ = 0;
    uint32_t cpu_core_num_;

private:
    ThreadPool(const ThreadPool&) = delete;
    ThreadPool(ThreadPool&&) = delete;
    void operator=(const ThreadPool&) = delete;
    void operator=(ThreadPool&&) = delete;
};

class StaticThreadPool final {
private:
    struct ThreadInfo final {
        pthread_t pid;
        uint32_t thread_idx;
        StaticThreadPool* pool;
    };

public:
    ~StaticThreadPool() {
        Destroy();
    }

    ppl::common::RetCode Init(uint32_t thread_num = 0);
    uint32_t GetNumThreads() const {
        return threads_.size();
    }
    void Destroy();

    void Run(const std::function<void(uint32_t nr_threads, uint32_t thread_idx)>& f);

    /** callers MUST make sure that the last call is finished before starting another new call */
    void RunAsync(const std::function<void(uint32_t nr_threads, uint32_t thread_idx)>& f);

    void Wait();

private:
    static void* ThreadWorker(void*);

private:
    std::vector<ThreadInfo> threads_;
    std::function<void(uint32_t nr_threads, uint32_t thread_idx)> func_;
    Barrier barrier_;
};

}}

#endif
