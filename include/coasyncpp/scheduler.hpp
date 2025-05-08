#ifndef __COASYNCPP_SCHEDULER_HPP__
#define __COASYNCPP_SCHEDULER_HPP__

#include "common.hpp"

#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <expected>

namespace coasyncpp
{
struct task_storage
{
    task_storage(async_interface *task) : task_{task}
    {
    }
    async_interface *task_;
    std::mutex mutex_{};
    std::condition_variable cv_{};
};

class Scheduler
{
  public:
    static Scheduler *getInstance()
    {
        if (nullptr == instance_)
            instance_ = new Scheduler();

        return instance_;
    }
    ~Scheduler()
    {
        isRunning_ = false;
        workerThread_.join();
    }

    void schedule(async_interface *task, bool blockThread = false)
    {
        // TODO: replace tasks_ with lock free one.
        auto ts = std::make_shared<task_storage>(task);
        std::unique_lock lock(ts->mutex_);
        {
            std::lock_guard tasksLock{tasksMutex_};
            tasks_.push(ts);
        }
        // Suspend thread
        if (blockThread)
            ts->cv_.wait(lock, [task]() { return task->done(); });
    }
    void resumeFromCallback(task_storage *taskStorage)
    {
        std::lock_guard lock{taskStorage->mutex_};
        taskStorage->cv_.notify_one();
    }

  private:
    static Scheduler *instance_;

    Scheduler()
    {
        isRunning_ = true;
        workerThread_ = std::thread(&Scheduler::worker, this);
    }

    std::queue<std::shared_ptr<task_storage>> tasks_{};
    bool isRunning_{};
    std::thread workerThread_{};
    std::mutex tasksMutex_{};

    void worker()
    {
        while (isRunning_)
        {
            while (isRunning_ && !tasks_.empty())
            {
                auto taskStorage = tasks_.front();
                {
                    std::lock_guard lock{tasksMutex_};
                    tasks_.pop();
                }

                if (taskStorage->task_->done())
                {
                    std::lock_guard lock{taskStorage->mutex_};
                    taskStorage->cv_.notify_one();
                }
                else
                {
                    {
                        std::lock_guard lock{tasksMutex_};
                        tasks_.push(taskStorage);
                    }
                    taskStorage->task_->execute();
                }
            }
            if (isRunning_)
                std::this_thread::yield();
        }
    }
};

Scheduler *Scheduler::instance_{};

// Tasks with callback support
template <typename T> struct awake_handle
{
    std::mutex mt_{};
    std::condition_variable cv_{};
    bool completed_{};

    std::expected<T, async_error> result_{};

    auto getResult() -> std::expected<T, async_error>
    {
        return result_;
    }

/*
    T value_{};

    T getValue() const
    {
        return value_;
    }
*/
};
template <> struct awake_handle<void>
{
    std::mutex mt_{};
    std::condition_variable cv_{};
    bool completed_{};

    std::expected<void, async_error> result_{};

    auto getResult() -> std::expected<void, async_error>
    {
        return result_;
    }
};

// Create task handle
template <typename T> awake_handle<T> *createTaskHandle()
{
    return new awake_handle<T>{};
}
template <> awake_handle<void> *createTaskHandle()
{
    return new awake_handle<void>{};
}

// Suspend from task
template <typename T> void suspend(awake_handle<T> *handle)
{
    std::unique_lock lock{handle->mt_};
    handle->cv_.wait(lock, [handle]() { return handle->completed_; });
}
template <> void suspend(awake_handle<void> *handle)
{
    std::unique_lock lock{handle->mt_};
    handle->cv_.wait(lock, [handle]() { return handle->completed_; });
}

// Resume from callback
template <typename T> void resume(T value, awake_handle<T> *handle)
{
    handle->completed_ = true;
    handle->result_ = value;
    std::lock_guard lock(handle->mt_);
    handle->cv_.notify_one();
}

template <typename T> void resume(int errorCode, char const *errorMessage, awake_handle<T> *handle)
{
    handle->completed_ = true;
    handle->result_ = std::unexpected(async_error{errorCode, errorMessage});
    std::lock_guard lock(handle->mt_);
    handle->cv_.notify_one();
}

void resume(awake_handle<void> *handle)
{
    handle->completed_ = true;
    std::lock_guard lock(handle->mt_);
    handle->cv_.notify_one();
}
void resume(int errorCode, char const *errorMessage, awake_handle<void> *handle)
{
    handle->completed_ = true;
    handle->result_ = std::unexpected(async_error{errorCode, errorMessage});
    std::lock_guard lock(handle->mt_);
    handle->cv_.notify_one();
}

} // namespace coasyncpp

#endif