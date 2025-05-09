# Coroutine async task based C++ library
![Coroutine async C++ library](https://github.com/rostislav-nikitin/coasyncpp/blob/main/icon-96.png?raw=true)<strong style="font-size: 200%; margin-left: -20px; vertical-align:baseline;">coasyncpp</strong>

## About
As you may know the coroutines support is one of the big four features of the C++20. Coroutines gives to the developer base mechanisms and abstractions to implement coperative multitasking.

The purpose of this library provide to the C++ developer easy to use semantics to build asynchronouse task based, functional, ranges supporting code like this:


```C++
#include <coasync/async.hpp>

#include <iostream>
#include <iomanip>
#include <utility>
#include <ranges>

using namespace coasync::core;

/// @bref Fibonachi numbers generation coroutine with automatical ranges sucpport.
auto fib() -> async<int>
{
    int n1{0};
    int n2{1};

    while (--startIndex)
        n1 = std::exchange(n2, n1 + n2);
    co_yield n1;

    while (--count){
        n1 = std::exchange(n2, n1 + n2);
        co_yield n1;
    }
}

/// @bref main function
auto main(int argc, char *argv[]) -> int
{
    /// Iterating over first 30 Fibonachi numbers starting from the first number.
    for (auto [index, n] : fib(1, 30) 
            // Filter out odd numbers
            | stdv::filter([](auto x) { return 1 == x % 2; }) 
            // Divide remaining even numbers by 2.0
            | stdv::transform([](auto x) { return double(x / 2.0); }) 
            // Enumerate resulting numbers
            | stdv::enumerate)
        // Output results
        std::cout 
            << std::setw(3) << (index + 1) 
            << "."
            << std::setw(12) << std::fixed << std::setprecision(2) << n
            << std::endl;

    return EXIT_SUCCESS;
}
```
Results:
```Text
  1.        0.50
  2.        0.50
  3.        1.50
  4.        2.50
  5.        6.50
  6.       10.50
  7.       27.50
  8.       44.50
  9.      116.50
 10.      188.50
 11.      493.50
 12.      798.50
 13.     2090.50
 14.     3382.50
 15.     8855.50
 16.    14328.50
 17.    37512.50
 18.    60696.50
 19.   158905.50
 20.   257114.50

```


## Build

## Use

## Examples

