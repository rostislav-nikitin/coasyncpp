#include <coasyncpp/async.hpp>

#include <cassert>
#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <memory>
#include <queue>
#include <chrono>

using namespace coasyncpp::expected;

/// The type that represents callback type accepted by the third party io library.
using callback_t = void (*)(int value, int errorCode, char const *errorMessage, void *user_data);
/// @brief The global variable that represents callback queue of the third party io library.
std::queue<std::tuple<int, callback_t, void *>> callbacksQueue;
/// @brief The global variable that represents a worker thread of the third party io library.
std::thread threadingPoolWorkerThread;
/// @brief The global variable thar inidicates does worker thread should countinue to run.
bool isRun{true};
/// @brief The function that represents third party io library worker thread function.
void threadingPoolWorker()
{
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    while (isRun)
    {
        while (!callbacksQueue.empty())
        {
            auto [id, callback, userData] = callbacksQueue.front();
            callbacksQueue.pop();

            if (10 == id)
                callback(50, 0, nullptr, userData);
            else
                callback(0, 1, "IO Error.", userData);
        }
    }
}
/// @brief The function that represents a third party io library thread pool start.
void startThreadPool()
{
    threadingPoolWorkerThread = std::thread(threadingPoolWorker);
}
/// @brief The function that represents a third party io library thread pool end.
void stopThreadPool()
{
    std::this_thread::sleep_for(std::chrono::milliseconds(5000));
    isRun = false;
        threadingPoolWorkerThread.join();
}
/// @brief The function that represents a part of the external C API of the third party library.
/// @param id The parameter that represents an id of some interesetd entity.
/// @param callback The parameter that represents a callback function that will called when job will done.
/// @param userData The parameter that represents a user data will be passed back via callback call.
void asyncFunc(int id, callback_t callback, void *userData)
{
    callbacksQueue.push(std::make_tuple(id, callback, userData));
}

/// @brief  The function that represents a user callback.
/// @param value The parameter that represents value recived from the third party io library.
/// @param userData The parameter that represents a used data passed back to the callback function.
void userCallback(int value, int errorCode, char const *errorMessage, void *userData)
{
    if(errorCode)
        resume(errorCode, errorMessage, static_cast<awake_handle<int> *>(userData));
    else
        resume(value, static_cast<awake_handle<int> *>(userData));
}

/// @brief The coroutine that calls third party io library async C API function.
/// @param id The parameter that represents an id of the entity.
/// @return Returns the async task.
auto ioTask(int id) -> async<int>
{
    std::unique_ptr<awake_handle<int>> handle{createTaskHandle<int>()};

    asyncFunc(id, userCallback, static_cast<void *>(handle.get()));
    suspend(handle.get());

    co_return handle->getResult();
}

/// @brief The coroutine that calls io one, calculate results and pass it back to the caller.
/// @param initial The parameter that represents some initial value.
/// @return Returns result of the calculations.
auto calculationTask(int initial) -> async<int>
{
    auto x = co_await ioTask(10);
    if(!x)
        co_return x;

    auto y = co_await ioTask(20);
    if(!y)
        co_return y;
    
    co_return initial + *x + *y;
}

auto main(int argc, char *argv[]) -> int
{
    // Start third io library thread pool.
    startThreadPool();

    // Create calculation task.
    auto task = calculationTask(10);
    /// Execute calculation task.
    task.execute();
    // Output calculation result.
    assert(!task);

    task.result()
        .and_then([](auto x) -> std::expected<int, async_error>
            {
                std::cout << x << std::endl;
                return x;
            })
        .or_else([](auto ex) -> std::expected<int, async_error>
            {
                std::cout << ex.what() << std::endl;
                return std::unexpected(ex);
            });

    // Stop third io library thread pool.
    stopThreadPool();

    return EXIT_SUCCESS;
}