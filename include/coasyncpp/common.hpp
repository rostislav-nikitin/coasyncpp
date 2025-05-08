#ifndef __COASYNCPP_COMMON_HPP__
#define __COASYNCPP_COMMON_HPP__

#include <coroutine>
#include <stdexcept>

namespace coasyncpp
{
/// @brief The interface that represents asyc task interface.
class async_interface
{
  public:
    virtual void execute() = 0;
    virtual bool done() = 0;
};
/// @brief The class that represents out of values sentinel.
struct async_sentinel
{
};

/// @brief The class that represents a resume awaiter.
/// @tparam T The template parameter that represents a concrete promise_type.
template <typename T> class resume_awaiter
{
  public:
    resume_awaiter(bool isAwaitReady) : isAwaitReady_{isAwaitReady}
    {
    }
    bool await_ready() noexcept
    {
        return isAwaitReady_;
    }
    std::coroutine_handle<> await_suspend(std::coroutine_handle<T> selfHandle) noexcept
    {
        return selfHandle.promise().callerHandle_;
    }
    void await_resume() noexcept
    {
    }

  private:
    bool isAwaitReady_{};
};
} // namespace coasyncpp

#endif