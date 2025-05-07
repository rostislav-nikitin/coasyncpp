#ifndef __COASYNCPP_COMMON_HPP__
#define __COASYNCPP_COMMON_HPP__

namespace coasyncpp
{
template <typename T> class async;

/// @brief The interface that represents asyc task interface.
class async_interface
{
  public:
    virtual void execute() = 0;
    virtual void suspend() = 0;
    virtual void awake() = 0;
    virtual bool done() = 0;
};
} // namespace coasyncpp

#endif