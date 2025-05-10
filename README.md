# Coroutine based async tasks C++ library
![Coroutine based async tasks C++ library](https://github.com/rostislav-nikitin/coasyncpp/blob/main/logo-5.png?raw=true)coasyncpp

## About
As you may know the coroutines support is one of the big four features added to the C++20. Coroutines provides to the developer base mechanisms and abstractions to implement a coperative multitasking.

The purpose of this library to provide to the C++ developer easy to use semantics to build asynchronous task based, functional code with support of the ranges.

But why we need it? To understand this let's look into the next examples.

### Problem

The first one is a C++ asynchronous API that encourages us to write spaghetti code like the one below.

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

Another example. Let's say we have a some asynchronous non-blocking C API. It consists of the bunch of the types and IO functions.

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

To use this API we should write not obvious code like below.
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
        ioWriteFunc(entityId, &ioWriteCallback, this);
    }
    void log(WriteResult &result)
    {
        // Logging results
    }
};
```

### Solution

But with use of the coroutines and coasyncpp library we can get much more readable code, wich represents all this asynchronouse code with a sequence of calls like a simple synchonous code like below.

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

So such approach makes high-level algorithm more readable and as result more maintainable and bug free.

But of course this solution also not absolutely perfect and has some of the boilerplate code.
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

/// @bref Fibonachi numbers generator coroutine with automatical ranges support.
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

/// @bref The main function
auto main(int argc, char *argv[]) -> int
{
    /// Iterating over the first 30 Fibonachi numbers starting from the first one
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

I hope this examples ispired you to get deeper into the coroutines and coasyncpp library in particular.

## Build

This is a header only library, so you can use it just by cloning repo and referencing header files.
```bash

git clone https://github.com/rostislav-nikitin/coasyncpp.git

```

But it you want to play with examples you can easely build them.

```bash

cd coasyncpp

mkdir build && cd build

cmake ..

cmake --build .

```

## Usage

