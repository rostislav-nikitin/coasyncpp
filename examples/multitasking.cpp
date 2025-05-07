#include <coasyncpp/async.hpp>

#include <cassert>
#include <iostream>
#include <ranges>

using namespace coasyncpp;
namespace stdv = std::ranges::views;

/// @brief The coroutine that generates odd or even numbers sequence.
/// @param start The parameter that specifies a first number in a sequence.
/// @param count The parameter that specifies a count of numbers to generate.
/// @param even The parameter that specifies what kind (odd/even) numbers to generate.
/// @return Returns a next odd or even number.
auto num(int start, int count, bool odd = true) -> async<int>
{
    assert(count > 0);
    start = (!odd == (start ? start : (start + 2)) % 2) ? start : (start + 1);

    while (count--)
    {
        std::cout << "Number: " << start << std::endl;
        co_yield start;
        start += 2;
    }
}

/// @brief The coroutine that finishes only when ALL tasks done.
auto whenAll() -> void
{
    auto taskOdds{num(0, 5, true)};
    auto taskEvens{num(0, 15, false)};

    auto task{whenAll(std::vector{taskOdds, taskEvens})};
    task.execute();
}

/// @brief The coroutine that finishes only when at least ANYONE tasks done.
auto whenAny() -> void
{
    auto taskOdds{num(0, 5, true)};
    auto taskEvens{num(0, 15, false)};

    auto task{whenAny(std::vector{taskOdds, taskEvens})};
    task.execute();
}

auto main(int argc, char *argv[]) -> int
{
    std::cout << "====================WHEN_ALL====================" << std::endl;
    whenAll();
    std::cout << "====================WHEN_ANY====================" << std::endl;
    whenAny();
 
    return EXIT_SUCCESS;
}