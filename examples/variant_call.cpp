#include <coasyncpp/async.hpp>

#include <stdexcept>
#include <expected>
#include <variant>
#include <iostream>

using namespace coasyncpp::variant;

/// @brief Returns valid result.
auto coWithSuccess() -> async<int, std::runtime_error, async_error>
{
    co_return 42;
}

/// @brief When uncaught exception then async_error returned as result.
auto coWithUncaughtRuntimeError() -> async<int, std::runtime_error, async_error>
{
    throw std::runtime_error("Something went wrong...");
	co_return 42;
}
/// @brief When uncaught non std::exception inherited exception then async_error returned as result.
auto coWithUncaughtUnsupportedError() -> async<int, std::runtime_error, async_error>
{
    throw 84;
	co_return 84;
}

/// @brief Catch and return any custom exception.
auto coWithCaughtRuntimeError() -> async<int, std::runtime_error, async_error>
{
    try
    {
        throw std::runtime_error("Something went wrong...");
	    co_return 42;
    }
    catch(const std::runtime_error& e)
    {
        co_return std::unexpected(e);
    }
}


auto runTask(async<int, std::runtime_error, async_error> &&task) -> void
{
    task.execute();
    auto result = task.result();

    using result_t = expected_result_t<int, std::runtime_error, async_error>;
    
    task.result()
        .and_then([](auto x) -> result_t {
            std::cout << x << std::endl;
            return x;
        })
        .or_else([](auto vex) -> result_t {
            std::visit([](auto &&ex) { std::cout << typeid(ex).name() << " : " << ex.what() << std::endl; }, vex);

            return std::unexpected(vex);
        });
}

auto main(int argc, char *argv[]) -> int
{
    runTask(coWithSuccess());
    runTask(coWithUncaughtRuntimeError());
    runTask(coWithUncaughtUnsupportedError());
    runTask(coWithCaughtRuntimeError());
}
