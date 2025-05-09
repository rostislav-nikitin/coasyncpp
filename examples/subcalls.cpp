#include <coasyncpp/async.hpp>

#include <cassert>
#include <iostream>
#include <coroutine>
#include <expected>
#include <variant>
#include <exception>
#include <stdexcept>

using namespace coasyncpp::variant;

/// @brief The parameterless inner coroutine.
/// @return Returns const value.
auto innerFunc() -> async<int, std::exception, async_error>
{
    //throw std::runtime_error("Number Error.");
    co_return 10;
    //co_return std::unexpected(async_error("RERROR"));
}

/// @brief The middle coroutine.
/// @param x The parameter that represents input value.
/// @return Returns result of the multiplication of the innerFunc result by x.
auto middleFunc(int x) -> async<int, std::exception, async_error>
{
    auto y = co_await innerFunc();


    if (!y)
        co_return y;

    co_return x **y;
}

/// @brief The mose outer coroutine.
/// @param x The parameter that represents input value.
/// @return Returns the sum of middleFunc result, another middleFunc result and x.
auto outerFunc(int x) -> async<int, std::exception, async_error>
{
    auto y = co_await middleFunc(x);
    if (!y)
        co_return y;

    auto z = co_await middleFunc(x);
    if (!z)
        co_return z;

    co_return x + *y + *z;
}

template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };


auto main(int argc, char *argv[]) -> int
{
    auto task{outerFunc(5)};
    task.execute();

    assert(task && (*task == (5 * 10 + 5 * 10 + 5)));

    using result_t = expected_result_t<int, std::exception, async_error>;

    
    task.result()
        .and_then([](auto x) -> result_t {
            std::cout << x << std::endl;
            return x;
        })
        .or_else([](auto vex) -> result_t {
            //std::cout << ex.what() << std::endl;
            /*
            std::visit(overloaded
            {
                [](std::exception ex){ std::cout << "std::exception.what(): " << ex.what(); },
                [](async_error ex){ std::cout << "async_error.what(): " << ex.what(); }
            }, ex);
            */
            std::visit([](auto &&ex) { std::cout << ex.what() << std::endl; }, vex);

            return std::unexpected(vex);
        });

    

    return EXIT_SUCCESS;
}