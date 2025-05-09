#include <coasyncpp/async.hpp>

#include <iostream>
#include <vector>

using namespace coasyncpp::core;

auto getVector() -> async<std::vector<int>>
{
    co_return std::vector<int>{1, 2, 3, 4, 5};
}

auto main(int argc, char *arvgv[]) -> int
{
    auto vec = getVector();
    std::cout << vec.result().size() << std::endl;


    return EXIT_SUCCESS;
}