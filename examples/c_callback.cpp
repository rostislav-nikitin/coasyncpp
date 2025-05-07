#include <coasyncpp/async.hpp>

#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <memory>
#include <queue>
#include <chrono>

using namespace coasyncpp;

using callback_t = void (*)(int value, void *user_data);

std::queue<std::pair<callback_t, void *>> callbacksQueue;
std::thread threadingPoolWorkerThread;
bool isRun{true};

void asyncFunc(int id, callback_t callback, void *userData)
{
    callbacksQueue.push(std::make_pair(callback, userData));
}

void threadingPoolWorker()
{
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    while (isRun)
    {
        while (!callbacksQueue.empty())
        {
            auto [callback, userData] = callbacksQueue.front();
            callbacksQueue.pop();

            callback(100, userData);
        }
    }
}

void startThreadPool()
{
    threadingPoolWorkerThread = std::thread(threadingPoolWorker);
}

void stopThreadPool()
{
    std::this_thread::sleep_for(std::chrono::milliseconds(5000));
    isRun = false;
    threadingPoolWorkerThread.join();
}

void userCallback(int value, void *userData)
{
    resume(value, static_cast<awake_handle<int> *>(userData));
}

auto ioTask(int id) -> async<int>
{
    std::unique_ptr<awake_handle<int>> handle{createTaskHandle<int>()};

    asyncFunc(id, userCallback, static_cast<void *>(handle.get()));
    suspend(handle.get());

    co_return handle->getValue();
}

auto calculationTask(int initial) -> async<int>
{
    int x = co_await ioTask(10);

    co_return initial + x;
}


auto main(int argc, char *argv[]) -> int
{
    startThreadPool();

    auto task = calculationTask(10);
    task.execute();
    
    std::cout << task.value() << std::endl;

    stopThreadPool();

    return EXIT_SUCCESS;
}