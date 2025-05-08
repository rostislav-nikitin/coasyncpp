#include <coasyncpp/async.hpp>

#include <iostream>
#include <exception>
#include <stdexcept>
#include <expected>
#include <type_traits>

using namespace coasyncpp;

auto co(bool isThrow = false) -> async<std::expected<int, std::runtime_error>>
{
    try
    {
        if(!isThrow)
            co_return std::expected<int, std::runtime_error>(10);
        else
            throw std::runtime_error("Error");
    }
    catch(const std::runtime_error & e)
    {
        co_return std::unexpected<std::runtime_error>(e);
    }
    
}

auto process(auto task)
{
    task.value()
        .and_then([](auto value)
            {
                std::cout << value << std::endl; 
                return std::expected<int, std::runtime_error>(value); 
            })
        .or_else([](auto &&ex) -> std::expected<int, std::runtime_error>
            {
                std::cout << ex.what() << std::endl;
                return std::unexpected<std::runtime_error>(ex); 
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