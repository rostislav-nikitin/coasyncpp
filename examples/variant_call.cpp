#include <coasyncpp/async.hpp>

#include <stdexcept>
#include <expected>
#include <variant>
#include <iostream>

using namespace coasyncpp::variant;

auto coWithSuccess() -> async<int, std::runtime_error, async_error>
{
    co_return 42;
}

auto coWithRuntimeError() -> async<int, std::runtime_error, async_error>
{
    throw std::runtime_error("Something went wrong...");
	co_return 42;
}

auto coWithUnsupportedError() -> async<int, std::runtime_error, async_error>
{
    throw 84;
	co_return 84;
}

auto runCo(async<int, std::runtime_error, async_error> &&task) -> void
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
            std::visit([](auto &&ex) { std::cout << ex.what() << std::endl; }, vex);

            return std::unexpected(vex);
        });
}

auto main(int argc, char *argv[]) -> int
{
    runCo(coWithSuccess());
    runCo(coWithRuntimeError());
    runCo(coWithUnsupportedError());
}
