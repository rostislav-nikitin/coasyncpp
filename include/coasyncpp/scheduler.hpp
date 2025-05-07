#ifndef __COASYNCPP_SCHEDULER_HPP__
#define __COASYNCPP_SCHEDULER_HPP__

#include "common.hpp"

#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>

namespace coasyncpp
{
struct TaskStorage
{
    TaskStorage(async_interface *task) : task_{task}
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
        auto ts = std::make_shared<TaskStorage>(task);
        std::unique_lock lock(ts->mutex_);
        {
            std::lock_guard tasksLock{tasksMutex_};
            tasks_.push(ts);
        }
        // Suspend thread
        if(blockThread)
            ts->cv_.wait(lock, [task]() { return task->done(); });
    }
/*/
    void scheduleNoBlocking(async_interface *task)
    {
        auto ts = std::make_shared<TaskStorage>(task);

        std::lock_guard tasksLock{tasksMutex_};
        tasks_.push(ts);
    }
*/
    void resumeFromCallback(TaskStorage *taskStorage)
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

    std::queue<std::shared_ptr<TaskStorage>> tasks_{};
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

} // namespace coasyncpp

#endif