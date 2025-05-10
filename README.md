# Coroutine based async tasks C++ library
![Coroutine based async tasks C++ library](https://github.com/rostislav-nikitin/coasyncpp/blob/main/logo.png?raw=true)

## About
As you may know, coroutines support is one of the big four features added to C++20. Coroutines provide developers a base mechanism and abstractions for implementing cooperative multitasking.

The library's purpose is to provide C++ developers easy-to-use semantics for building asynchronous task-based, functional code with support for ranges.

But why do we need it? To understand this let's look at the next examples.

### Problem

The first is a C++ asynchronous API that encourages us to write spaghetti code like the one below.

```C++
int main(int argc, char* arg[])
{
    // Read some entity
    ioReadTask(int id, [](SomeEntity &entity)
    {
        // Some business logic
        // ...
        // Write result back
        ioWriteTask(entity, [](WriteResult &result)
        {
            // Some result processing
            ioWriteAnotherTask(result, [](AnotherWriteResult &resule)
            {
                //...
                    //...
                        //...
                            log();
            })
        })
    });
}
```

Another example. Let's say we have some asynchronous non-blocking C API. It consists of a bunch of types and IO functions.

```C++
/// @brief Some arbitraty entity structure.
struct Entity
{
    //...
};

// @brief Read callback type.
using read_callback_t = void(*)(Entity &entity, void *userData);

// @brief Write callback type.
using write_callback_t = void(*)(WriteResult &result, void *userData);

/// @brief The function that represents a part of the external C API of the third party IO library.
void ioReadFunc(int entityId, read_callback_t callback, void *userData)
{
    // Register some task to run read IO in the separater thread
    // When task done -- run callback
}

/// @brief The function that represents a part of the external C API of the third party IO library.
void ioWriteFunc(Entity* entity, write_callback_t callback, void *userData)
{
    // Register some task to run write IO in the separater thread
    // When task done -- run callback
}
```

To use this API, we should write non-obvious code like below.
```C++

class Controller;

/// @brief  The function that represents an API user read callback.
void ioReadCallback(Entity &entity, void *userData)
{
    Controller *controller = static_cast<Controller *>(userData);
    controller.process(entity);
}

/// @brief  The function that represents an API user write callback.
void ioWriteCallback(WriteResult &result, void *userData)
{
    Controller *controller = static_cast<Controller *>(userData);
    controller.log(result);
}

class Controller
{
public:
    void read(int entityId)
    {
        ioReadFunc(entityId, &ioReadCallback, this);
    }
    void process(Entity &entity)
    {
        // Do some processing...
        // Save entity
        ioWriteFunc(entity, &ioWriteCallback, this);
    }
    void log(WriteResult &result)
    {
        // Log results
    }
};
```

### Solution

But with the use of the coroutines and coasyncpp library, we can get much more readable code, which represents all this asynchronous code with a sequence of calls like a simple synchronous code, as shown below.

```C++
auto process(int entityId) -> asyc<void>
{
    auto entity = co_await ioReadTask(entutyId);
    auto result = co_await ioWriteTask(entity);

    log(result);
}

auto main(int argc, char *argv[])
{
    using reult_t = std::expected<int, async_error>;

    int entityId = 42;
    auto task = process(entityId);
    task.exeute();
    task.result()
        // Normal path
        .and_then([](auto x) -> result_t
            {
                std::cout << x << std::endl;
                return x;
            })
        // Error path
        .or_else([](auto ex) -> result_t
            {
                std::cout << ex.what() << std::endl;
                return std::unexpected(ex);
            });

}
```

So such an approach makes a high-level algorithm more readable and, as a result, more maintainable and bug-free.

But of course, this solution is also not perfect and has some boilerplate code.
```C++
/// @brief  The function that represents a user callback.
void ioReadCallback(Entity &entity, void *userData)
{
    resume(entity, static_cast<awake_handle<int> *>(userData));
}

/// @brief The coroutine that calls third party IO library async C API function.
auto ioReadTask(int id) -> async<int>
{
    std::unique_ptr<awake_handle<int>> handle{createTaskHandle<int>()};
    ioReadFunc(id, ioReadCallback, static_cast<void *>(handle.get()));
    suspend(handle.get());

    co_return handle->getResult();
}

/// @brief  The function that represents a user callback.
void ioWriteCallback(WriteResult &result, void *userData)
{
    resume(result, static_cast<awake_handle<int> *>(userData));
}

/// @brief The coroutine that calls third party IO library async C API function.
auto ioWriteTask(int id) -> async<int>
{
    std::unique_ptr<awake_handle<int>> handle{createTaskHandle<int>()};
    ioWriteFunc(id, ioWriteCallback, static_cast<void *>(handle.get()));
    suspend(handle.get());

    co_return handle->getResult();
}
```

The next piece of code shows how coasyncpp can be used to generate sequences of values.

```C++
#include <coasync/async.hpp>

#include <iostream>
#include <iomanip>
#include <utility>
#include <ranges>

using namespace coasync::core;

/// @brief Fibonacci numbers generator coroutine with automatic range support.
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

/// @brief The main function.
auto main(int argc, char *argv[]) -> int
{
    /// Iterating over the first 30 Fibonacci numbers starting from the first one
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

I hope those examples inspired you to get deeper into the coroutines and coasyncpp library in particular.

## Build

This is a header-only library, so you can use it just by cloning the repo and referencing the header files.

```bash

git clone https://github.com/rostislav-nikitin/coasyncpp.git

```

But if you want to play with examples, you can easily build them.

```bash

cd coasyncpp

mkdir build && cd build

cmake ..

cmake --build .

```

## Usage

The library provides three gradations of the coroutines: core, expected, and variant. 
The only difference between them is the return type.

### Core

Core functionality is represented in the `coasyncpp::core` namespace.

Let's look at the next example:

```C++
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
```

```
42
```

So, in the core implementation, coroutine result type is an async<T> where the type of the task.result() is T.

### Expected

Expected functionality is represented in the `coasyncpp::expected` namespace.

Let's look at the next example:

```C++
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
```

```
42
Something went wrong...
```

As you can see in the expected implementation coroutine result type is an `async<T>` where the type of the `task.result()` is a `std::expected<T, async_error>`. 

- In the successful case, the resulting value will be wrapped into the `std::expected` value member
- But if some uncaught exception is thrown, then the error member of std::excpected will store the `async_error`.

So, all uncaught exceptions that happen inside a coroutine are caught in the background and transformed into the `async_error`.

### Variant

Variant functionality is represented in the `coasyncpp::variant` namespace.

Let's look at the next example:

```C++
#include <coasyncpp/async.hpp>

#include <stdexcept>
#include <expected>
#include <variant>
#include <iostream>

using namespace coasyncpp::variant;

/// @brief Returns valid result.
auto coWithSuccess() -> async<int, std::runtime_error, async_error>
{
    co_return 42;
}

/// @brief When uncaught exception then async_error returned as result.
auto coWithUncaughtRuntimeError() -> async<int, std::runtime_error, async_error>
{
    throw std::runtime_error("Something went wrong...");
	co_return 42;
}
/// @brief When uncaught non std::exception inherited exception then async_error returned as result.
auto coWithUncaughtUnsupportedError() -> async<int, std::runtime_error, async_error>
{
    throw 84;
	co_return 84;
}

/// @brief Catch and return any custom exception.
auto coWithCaughtRuntimeError() -> async<int, std::runtime_error, async_error>
{
    try
    {
        throw std::runtime_error("Something went wrong...");
	    co_return 42;
    }
    catch(const std::runtime_error& e)
    {
        co_return std::unexpected(e);
    }
}


auto runTask(async<int, std::runtime_error, async_error> &&task) -> void
{
    task.execute();
    auto result = task.result();

    using result_t = expected_result_t<int, std::runtime_error, async_error>;
    
    task.result()
        .and_then([](auto x) -> result_t {
            std::cout << x << std::endl;
            return x;
        })
        .or_else([](auto vex) -> result_t {
            std::visit([](auto &&ex) { std::cout << typeid(ex).name() << " : " << ex.what() << std::endl; }, vex);

            return std::unexpected(vex);
        });
}

auto main(int argc, char *argv[]) -> int
{
    runTask(coWithSuccess());
    runTask(coWithUncaughtRuntimeError());
    runTask(coWithUncaughtUnsupportedError());
    runTask(coWithCaughtRuntimeError());
}
```

```
N9coasyncpp11async_errorE : Something went wrong...
N9coasyncpp11async_errorE : Unknown error.
St13runtime_error : Something went wrong...
```

IIn the variant implementation coroutine result type is an `async<T, E1, E2, ...>` where the type of the `task.result()` is a `std::expected<T, std::variant<E1, E2, ...>>`. `E1, E2, ...` are the types of errors that can happen via coroutine call. 

- In the successful case, the resulting value will be wrapped into the `std::expected` value member
- If any uncaught exception is thrown (inherited from std::exception or not), then the error member of std::excpected will store the `std::variant<E1, E2, ...>` with `async_error`
- If you want to put some typed error different from the `async_error`, then you need to catch an exception in the coroutine code and return it via std::unexpected.

So, all uncaught exceptions inside a coroutine are caught in the background and transformed into the `async_error`, and all other exceptions can be caught and returned via std::unexpected.