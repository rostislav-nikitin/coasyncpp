#ifndef __COASYNCPP_COMMON_HPP__
#define __COASYNCPP_COMMON_HPP__

#include <coroutine>
#include <stdexcept>
#include <cstring>

namespace coasyncpp
{
/// @brief The interface that represents asyc task interface.
class async_interface
{
  public:
    virtual void execute() = 0;
    virtual bool done() = 0;
    virtual ~async_interface() { }
};

/// @brief The class that represents an aync error.
class async_error : public std::runtime_error
{
  public:
    async_error(char const *msg) : async_error(0, msg)
    {
    }

    async_error(int code, char const *msg) : code_{code}, runtime_error(msg)
    {
    }

    int code()
    {
        return code_;
    }

  private:
    int code_{};
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
    resume_awaiter(bool isFromStackCall) : isFromStackCall_{isFromStackCall}
    {
    }
    bool await_ready() noexcept
    {
        return false;
    }
    std::coroutine_handle<> await_suspend(std::coroutine_handle<T> selfHandle) noexcept
    {
        if(isFromStackCall_)
          return std::noop_coroutine();
        else
          return selfHandle.promise().callerHandle_;
    }
    void await_resume() noexcept
    {
    }

  private:
    bool isFromStackCall_{};
};

void coroutineHandleDestroyer(std::coroutine_handle<> handle)
{
  handle.destroy();
}
} // namespace coasyncpp

#endif