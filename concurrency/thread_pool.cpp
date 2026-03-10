#include <iostream>
#include <array>
#include <atomic>
#include <concepts>
#include <condition_variable>

#include <functional>

#include <memory>
#include <mutex>
#include <queue>
#include <thread>

namespace {
    constexpr std::size_t MAX_THREADS = 32;
}

template <typename FTask,  std::size_t N>
requires (N <= MAX_THREADS)
class ThreadPool
                : public std::enable_shared_from_this<ThreadPool<FTask, N>> {

    std::array<std::jthread, N> threads_;
    // std::pair<FTask, Args...> 
    std::queue<FTask> tasks_;
    mutable std::mutex tasks_mutex_;
    std::condition_variable cv_;
public:
    ThreadPool() = default;
    ~ThreadPool() {
        for (auto& th : threads_) {
            th.request_stop();
        }
        cv_.notify_all();
    }
    void Run() {
        auto self = this->weak_from_this();

        for (auto& th : threads_) {
            th = std::jthread([self](std::stop_token st) {
                if (auto strong = self.lock()) {
                    strong->worker(st);
                }
            });
        }
    }

    template <typename... Args>
    requires std::invocable<FTask, Args...>
    void AddTask(FTask func/*, Args&&... args*/) {
        {
            std::scoped_lock lock(tasks_mutex_);
            tasks_.push(func);
        }
        cv_.notify_one();
    }

private:
    void worker(std::stop_token stoken) {

        while (true) {
            FTask current_task;
            {
                std::unique_lock lock(tasks_mutex_);
                // enable_shared_from_this
                cv_.wait(lock, [this, &stoken] { 
                    return stoken.stop_requested() || !this->tasks_.empty(); 
                });

                if (stoken.stop_requested())
                    return;

                if (this->tasks_.empty())
                    continue;

                current_task = tasks_.front();
                tasks_.pop();
                //std::cout << tasks_.size() << std::endl;
            }

            current_task();
        }
    }
};

int main() {
    {
        std::atomic<int> i = 0;
        auto incr = [&i](){ ++i; };
        std::shared_ptr<ThreadPool<std::function<void()>, 4>> pool = std::make_shared<ThreadPool<std::function<void()>, 4>>();
        for (int i = 0; i < 1'000'000; ++i)
        {
            pool->AddTask(incr);
        }
        pool->Run();
        std::this_thread::sleep_for(std::chrono::seconds(5));
        
        std::cout << "result: " << i << std::endl;
    }  
}