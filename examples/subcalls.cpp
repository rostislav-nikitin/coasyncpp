#include <coasyncpp/async.hpp>

#include <cassert>
#include <iostream>
#include <coroutine>
#include <expected>

using namespace coasyncpp::expected;

/// @brief The parameterless inner coroutine.
/// @return Returns const value.
auto innerFunc() -> async<int>
{
    co_return 10;
}

/// @brief The middle coroutine.
/// @param x The parameter that represents input value.
/// @return Returns result of the multiplication of the innerFunc result by x.
auto middleFunc(int x) -> async<int>
{
    auto y = co_await innerFunc();

    if (!y)
        co_return y;

    co_return x **y;
}

/// @brief The mose outer coroutine.
/// @param x The parameter that represents input value.
/// @return Returns the sum of middleFunc result, another middleFunc result and x.
auto outerFunc(int x) -> async<int>
{
    auto y = co_await middleFunc(x);
    auto z = co_await middleFunc(x);

    if (!y)
        co_return y;
    if (!z)
        co_return z;

    co_return x + *y + *z;
}

auto main(int argc, char *argv[]) -> int
{
    auto task{outerFunc(5)};
    task.execute();
    task.result()
        .and_then([](auto x) -> std::expected<int, async_error> {
            std::cout << x << std::endl;
            return x;
        })
        .or_else([](auto ex) -> std::expected<int, async_error> {
            std::cout << ex.what() << std::endl;
            return std::unexpected(ex);
        });

    assert(task && (*task == (5 * 10 + 5 * 10 + 5)));

    return EXIT_SUCCESS;
}