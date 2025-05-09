#ifndef __COASYNCPP_ASYNC_CORE_HPP__
#define __COASYNCPP_ASYNC_CORE_HPP__

#include "common.hpp"
#include "scheduler.hpp"

#include <coroutine>
#include <iterator>
#include <vector>

/// @brief The namespace that represents classes/functions for async tasks manipulation.
namespace coasyncpp
{
namespace core
{
using namespace coasyncpp;

template <typename T> class async;

/// @brief The class that represents iterator for async task.
/// @tparam T The type of the iterator value.
template <typename T> class async_iterator
{
  public:
    using iterator_category = std::forward_iterator_tag;
    using difference_type = int;
    using value_type = T;
    using pointer = T *;
    using reference = T &;

    async_iterator(async<T> *task) : task_{task}
    {
    }

    T operator*() const
    {
        return task_->result();
    }
    async_iterator &operator++()
    {
        task_->execute();

        return *this;
    }
    async_iterator operator++(int)
    {
        auto temp = *this;

        task_->execute();

        return temp;
    }

    bool done() const
    {
        return task_->done();
    }

    friend bool operator==<T>(async_iterator const &lh, async_iterator const &rh);
    friend bool operator!=<T>(async_iterator const &lh, async_iterator const &rh);
    friend bool operator==<T>(async_iterator const &i, async_sentinel const &s);
    friend bool operator!=<T>(async_iterator const &i, async_sentinel const &s);
    friend bool operator==<T>(async_sentinel const &s, async_iterator const &i);
    friend bool operator!=<T>(async_sentinel const &s, async_iterator const &i);

  private:
    async<T> *task_{};
    mutable bool nextDone_{};
};

template <typename T> bool operator==(async_iterator<T> const &lh, async_iterator<T> const &rh)
{
    return lh.value() == rh.value();
}
template <typename T> bool operator!=(async_iterator<T> const &lh, async_iterator<T> const &rh)
{
    return !(lh == rh);
}
bool operator==(async_sentinel const &lh, async_sentinel const &rh)
{
    return true;
}
bool operator!=(async_sentinel const &lh, async_sentinel const &rh)
{
    return !(lh == rh);
}

template <typename T> bool operator==(async_iterator<T> const &i, async_sentinel const &s)
{
    return i.done();
}
template <typename T> bool operator!=(async_iterator<T> const &i, async_sentinel const &s)
{
    return !(i == s);
}
template <typename T> bool operator==(async_sentinel const &s, async_iterator<T> const &i)
{
    return i == s;
}
template <typename T> bool operator!=(async_sentinel const &s, async_iterator<T> const &i)
{
    return !(s == i);
}

/// @brief The class that represents async task.
/// @tparam T The type of the async task value.
template <typename T> class async : public async_interface
{
  public:
    // Promise type of the Self Result
    struct promise_type
    {
        std::suspend_always initial_suspend()
        {
            return {};
        }
        resume_awaiter<promise_type> final_suspend() noexcept
        {
            isDone_ = true;
            return {isFromStackCall_};
        }
        std::suspend_always return_value(T value)
        {
            value_ = value;
            return {};
        }
        std::suspend_always yield_value(T value)
        {
            value_ = value;
            return {};
        }
        void unhandled_exception()
        {
        }
        auto get_return_object()
        {
            return async<T>(std::coroutine_handle<promise_type>::from_promise(*this));
        }

        T value_{};
        std::coroutine_handle<> callerHandle_;
        bool isFromStackCall_{true};
        bool isDone_{};
    };

    // Awaiter members
    bool await_ready()
    {
        return false;
    }
    void await_suspend(std::coroutine_handle<> callerHandle)
    {
        selfHandle_->promise().callerHandle_ = callerHandle;
        selfHandle_->promise().isFromStackCall_ = false;
        selfHandle_->resume();
    }
    T await_resume()
    {
        return selfHandle_->promise().value_;
    }

    // Members
    async(std::coroutine_handle<promise_type> selfHandle) :
        selfHandle_{new std::coroutine_handle<promise_type>{selfHandle},
        [](std::coroutine_handle<promise_type> *handlePtr)
        {
            handlePtr->destroy();
            delete handlePtr;
        }}

    {
    }

    void execute() override
    {
        if (!selfHandle_->done())
            selfHandle_->resume();
    }
    bool done() override
    {
        return selfHandle_->promise().isDone_;
    }

    async_iterator<T> begin()
    {
        execute();
        return async_iterator{this};
    }
    async_sentinel end()
    {
        return {};
    }

    T operator*() const
    {
        return selfHandle_->promise().value_;
    }
    T result() const
    {
        return selfHandle_->promise().value_;
    }

  protected:
  private:
    std::shared_ptr<std::coroutine_handle<promise_type>> selfHandle_{};
};

/// @brief The class that represents async task.
/// @tparam T The type of the async task value.
template <> class async<void> : public async_interface
{
  public:
    // Promise type of the Self Result
    struct promise_type
    {
        std::suspend_always initial_suspend()
        {
            return {};
        }
        resume_awaiter<promise_type> final_suspend() noexcept
        {
            isDone_ = true;
            return {isFromStackCall_};
        }
        std::suspend_always return_void()
        {
            return {};
        }
        void unhandled_exception()
        {
        }
        auto get_return_object()
        {
            return async<void>(std::coroutine_handle<promise_type>::from_promise(*this));
        }

        std::coroutine_handle<> callerHandle_;
        bool isFromStackCall_{true};
        bool isDone_{};
    };

    // Awaiter members
    bool await_ready()
    {
        return false;
    }
    void await_suspend(std::coroutine_handle<> callerHandle)
    {
        selfHandle_->promise().callerHandle_ = callerHandle;
        selfHandle_->promise().isFromStackCall_ = false;
        selfHandle_->resume();
    }
    void await_resume()
    {
    }

    // Members
    async(std::coroutine_handle<promise_type> selfHandle) : 
        selfHandle_{new std::coroutine_handle<promise_type>{selfHandle},
        [](std::coroutine_handle<promise_type> *handlePtr)
        {
            handlePtr->destroy();
            delete handlePtr;
        }}
    {
    }


    void execute() override
    {
        if (!selfHandle_->done())
            selfHandle_->resume();
    }
    bool done() override
    {
        return selfHandle_->promise().isDone_;
    }

  protected:
  private:
    std::shared_ptr<std::coroutine_handle<promise_type>> selfHandle_{};
};

template <typename T> async<void> whenAll(std::vector<async<T>> tasks)
{
    for (auto &task : tasks)
        Scheduler::getInstance()->schedule(&task);

    for (auto task : tasks)
    {
        while (!task.done())
            std::this_thread::yield();
    }
    co_return;
}

template <typename T> async<void> whenAny(std::vector<async<T>> tasks)
{
    for (auto &task : tasks)
        Scheduler::getInstance()->schedule(&task);

    while (true)
    {
        for (auto task : tasks)
        {
            if (task.done())
                co_return;
            std::this_thread::yield();
        }
    }
}
} // namespace core
} // namespace coasyncpp

#endif