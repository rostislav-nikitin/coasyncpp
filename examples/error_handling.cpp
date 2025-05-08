#include <coasyncpp/async.hpp>

#include <iostream>
#include <exception>
#include <stdexcept>
#include <expected>

using namespace coasyncpp::expected;

auto co(bool isThrow = false) -> async<int>
{
    if(!isThrow)
        co_return 10;
    else
        throw std::runtime_error("Some Runtime Error.");
}

auto process(auto task)
{
    task.result()
        .transform_error([](auto ex) -> std::runtime_error
            {
                return std::runtime_error(ex.what());
            })
        .transform([](auto x) { return x * x; })
        .and_then([](auto value) -> std::expected<int, std::runtime_error>
            {
                std::cout << value << std::endl; 
                return value; 
            })
        .or_else([](auto ex) -> std::expected<int, std::runtime_error>
            {
                std::cout << ex.what() << std::endl;
                return std::unexpected<std::runtime_error>(ex.what()); 
            });
}

auto main(int argc, char *argv[]) -> int
{
    std::cout << "====================NOT THROWING====================" << std::endl;
    auto notmalTask = co(false);
    notmalTask.execute();
    process(notmalTask);

    std::cout << "======================THROWING======================" << std::endl;
    auto throwingTask = co(true);
    throwingTask.execute();
    process(throwingTask);

    
    return EXIT_SUCCESS;
}