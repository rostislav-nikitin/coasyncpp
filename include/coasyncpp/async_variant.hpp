#ifndef __COASYNCPP_ASYNC_VARIANT_HPP__
#define __COASYNCPP_ASYNC_VARIANT_HPP__

#include "common.hpp"
#include "scheduler.hpp"

#include <exception>
#include <stdexcept>
#include <coroutine>
#include <iterator>
#include <expected>
#include <variant>
#include <vector>
#include <cstring>
#include <iostream>

/// @brief The namespace that represents classes/functions for async tasks manipulation.
namespace coasyncpp
{
namespace variant
{
using namespace coasyncpp;

template <typename T, typename... Es> class async;

/// @bref The type that represents value type.
template <typename T, typename... Es> using expected_result_t = std::expected<T, std::variant<Es...>>;

/// @brief The class that represents iterator for async task.
/// @tparam T The type of the iterator value.
template <typename T, typename... Es> class async_iterator
{
  public:
    using iterator_category = std::forward_iterator_tag;
    using difference_type = int;
    using value_type = expected_result_t<T, Es...>;
    using pointer = expected_result_t<T, Es...> *;
    using reference = expected_result_t<T, Es...> &;

    async_iterator(async<T, Es...> *task) : task_{task}
    {
    }

    expected_result_t<T, Es...> operator*() const
    {
        return task_->value();
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

    friend bool operator==<T, Es...>(async_iterator const &lh, async_iterator const &rh);
    friend bool operator!=<T, Es...>(async_iterator const &lh, async_iterator const &rh);
    friend bool operator==<T, Es...>(async_iterator const &i, async_sentinel const &s);
    friend bool operator!=<T, Es...>(async_iterator const &i, async_sentinel const &s);
    friend bool operator==<T, Es...>(async_sentinel const &s, async_iterator const &i);
    friend bool operator!=<T, Es...>(async_sentinel const &s, async_iterator const &i);

  private:
    async<T, Es...> *task_{};
    mutable bool nextDone_{};
};

template <typename T, typename... Es> bool operator==(async_iterator<T, Es...> const &lh, async_iterator<T, Es...> const &rh)
{
    return lh.value() == rh.value();
}
template <typename T, typename... Es> bool operator!=(async_iterator<T, Es...> const &lh, async_iterator<T, Es...> const &rh)
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

template <typename T, typename... Es> bool operator==(async_iterator<T, Es...> const &i, async_sentinel const &s)
{
    return i.done();
}
template <typename T, typename... Es> bool operator!=(async_iterator<T, Es...> const &i, async_sentinel const &s)
{
    return !(i == s);
}
template <typename T, typename... Es> bool operator==(async_sentinel const &s, async_iterator<T, Es...> const &i)
{
    return i == s;
}
template <typename T, typename... Es> bool operator!=(async_sentinel const &s, async_iterator<T, Es...> const &i)
{
    return !(s == i);
}

/// @brief The class that represents async task.
/// @tparam T The type of the async task value.
template <typename T, typename... Es> class async : public async_interface
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
        std::suspend_always return_value(expected_result_t<T, Es...> value)
        {
            value_ = value;
            return {};
        }
        std::suspend_always yield_value(expected_result_t<T, Es...> value)
        {
            value_ = value;
            return {};
        }
        // void return_void() { isDone_ = true; }
        void unhandled_exception()
        {
            std::exception_ptr ePtr{std::current_exception()};
            if (ePtr)
            {
                try
                {
                    std::rethrow_exception(ePtr);
                }
                catch (const std::exception &e)
                {
                    value_ = std::unexpected(std::variant<Es...>{e});
                }
                catch (...)
                {
                    value_ = std::unexpected(std::variant<Es...>(async_error("Unknown error.")));
                }
            }
        }
        auto get_return_object()
        {
            return async<T, Es...>(std::coroutine_handle<promise_type>::from_promise(*this));
        }


        expected_result_t<T, Es...> value_{};
        std::coroutine_handle<> callerHandle_{};
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
    expected_result_t<T, Es...> await_resume()
    {
        return selfHandle_->promise().value_;
    }

    // Members
    async(std::coroutine_handle<promise_type> selfHandle) :
        selfHandle_
        {
            new std::coroutine_handle<promise_type>{selfHandle},
            [](std::coroutine_handle<promise_type> *handlePtr)
            {
                handlePtr->destroy();
                delete handlePtr;
            }
        }
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

    async_iterator<T, Es...> begin()
    {
        execute();
        return async_iterator{this};
    }
    async_sentinel end()
    {
        return {};
    }
    operator bool() const
    {
        return selfHandle_->promise().value_.has_value();
    }
    expected_result_t<T, Es...> operator*()
    {
        return selfHandle_->promise().value_;
    }
    expected_result_t<T, Es...> result()
    {
        return selfHandle_->promise().value_;
    }

  protected:
  private:
    std::shared_ptr<std::coroutine_handle<promise_type>> selfHandle_{};
};

/// @brief The class that represents async task.
/// @tparam T The type of the async task value.
template <typename... Es> class async<void, Es...> : public async_interface
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
            std::exception_ptr ePtr{std::current_exception()};
            if (ePtr)
            {
                try
                {
                    std::rethrow_exception(ePtr);
                }
                catch (const std::exception &e)
                {
                    //value_ = std::unexpected(async_error(e.what()));
                    value_ = std::unexpected(std::variant<Es...>(e));
                }
                catch (...)
                {
                    value_ = std::unexpected(std::variant<Es...>(async_error("Unknown error.")));
                }
            }
        }
        auto get_return_object()
        {
            return async<void, Es...>(std::coroutine_handle<promise_type>::from_promise(*this));
        }

        expected_result_t<void, Es...> value_{};
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
        selfHandle_
        {
            new std::coroutine_handle<promise_type>{selfHandle},
            [](std::coroutine_handle<promise_type> *handlePtr)
            {
                handlePtr->destroy();
                delete handlePtr;
            }
        }
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
    operator bool() const
    {
        return selfHandle_->promise().value_.has_value();
    }
    expected_result_t<void, Es...> operator*()
    {
        return selfHandle_->promise().value_;
    }
    expected_result_t<void, Es...> result()
    {
        return selfHandle_->promise().value_;
    }

  protected:
  private:
    std::shared_ptr<std::coroutine_handle<promise_type>> selfHandle_{};
};

template <typename T, typename... Es> async<void, Es...> whenAll(std::vector<async<T, Es...>> tasks)
{
    for (auto &task : tasks)
        Scheduler::getInstance()->schedule(&task);

    for (auto &task : tasks)
    {
        while (!task.done())
            std::this_thread::yield();
    }

    co_return;
}

template <typename T, typename... Es> async<void, Es...> whenAny(std::vector<async<T, Es...>> tasks)
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
} // namespace expected
} // namespace coasyncpp

#endif