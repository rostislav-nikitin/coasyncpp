#include <coasyncpp/async.hpp>

#include <cassert>
#include <iostream>
#include <coroutine>

using namespace coasyncpp;

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
    int y = co_await innerFunc();

    co_return x * y;
}

/// @brief The mose outer coroutine.
/// @param x The parameter that represents input value.
/// @return Returns the sum of middleFunc result, another middleFunc result and x.
auto outerFunc(int x) -> async<int>
{
    int y = co_await middleFunc(x);
    int z = co_await middleFunc(x);

    co_return x + y + z;
}

auto main(int argc, char* argv[]) -> int
{
    auto task{outerFunc(5)};
    task.execute();

    assert(task.value() ==  (5 * 10 + 5 * 10 + 5));
    std::cout << "Result value is: " << task.value() << std::endl;

    return EXIT_SUCCESS;
}