#include <coasyncpp/async.hpp>

#include <cassert>
#include <cstdint>
#include <iostream>
#include <coroutine>
#include <ranges>
#include <utility>
#include <iomanip>
#include <expected>

using namespace coasyncpp::core;

namespace stdv = std::ranges::views;

/// @brief The coroutine that generates Fibonacci numbers.
/// @param startIndex The parameter that represents index of the first Fibonacci number to generate. Numbering stating
/// from the 1.
/// @param count The parameter that represents the count of numbers to generate. Should be greater then 0.
/// @return Returns the next Fibonacci number.
auto fib(int startIndex, int count) -> async<u_int64_t>
{
    assert(startIndex > 0);
    assert(count > 0);

    uint64_t n1{0};
    uint64_t n2{1};

    while (--startIndex)
    {
        n1 = std::exchange(n2, n1 + n2);
    }

    co_yield n1;

    while (--count)
    {
        n1 = std::exchange(n2, n1 + n2);

        co_yield n1;
    }
}

int main(int argc, char *argv[])
{
    for (auto [index, n] : fib(1, 30) 
            | stdv::filter([](auto x) { return 1 == x % 2; }) 
            | stdv::transform([](auto x) { return double(x / 2.0); }) 
            | stdv::enumerate)
        std::cout 
            << std::setw(3) << (index + 1) 
            << "."
            << std::setw(12) << std::fixed << std::setprecision(2) << n
            << std::endl;

    return EXIT_SUCCESS;
}