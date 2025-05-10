#include <coasyncpp/async.hpp>

#include <stdexcept>
#include <expected>
#include <iostream>

using namespace coasyncpp::expected;

auto coWithSuccess() -> async<int>
{
    co_return 42;
}

auto coWithError() -> async<int>
{
    throw std::runtime_error("Something went wrong...");
	co_return 42;
}

auto main(int argc, char *argv[]) -> int
{
    // Success case
    async<int> taskWithSuccess = coWithSuccess();
    taskWithSuccess.execute();

    std::expected<int, async_error> resultWithSuccess = taskWithSuccess.result();

    if(resultWithSuccess.has_value())
        std::cout << resultWithSuccess.value() << std::endl;
    else
        std::cout << resultWithSuccess.error().what() << std::endl;

    // Error caase
    async<int> taskWithError = coWithError();
    taskWithError.execute();

    std::expected<int, async_error> resultWithError = taskWithError.result();

    if(resultWithError.has_value())
        std::cout << resultWithError.value() << std::endl;
    else
        std::cout << resultWithError.error().what() << std::endl;
}

