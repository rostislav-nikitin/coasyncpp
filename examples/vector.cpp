#include <coasyncpp/async.hpp>

#include <iostream>
#include <vector>

using namespace coasyncpp::core;

auto getVector() -> async<std::vector<int>>
{
    std::vector<int> vec{1, 2, 3, 4, 5};

    co_return vec;
}

auto main(int argc, char *arvgv[]) -> int
{
    auto vec = getVector();
    vec.execute();
    
    std::cout << vec.result().size() << std::endl;

    return EXIT_SUCCESS;
}