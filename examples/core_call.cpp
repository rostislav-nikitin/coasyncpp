#include <coasyncpp/async.hpp>

#include <iostream>

using namespace coasyncpp::core;

auto co() -> async<int>
{
    co_return 42;
}

auto main(int argc, char *argv[]) -> int
{
    async<int> task = co();
    task.execute();

    int result = task.result();

    std::cout << result << std::endl;
}
