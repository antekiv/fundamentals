#include <iostream>
#include <array>
#include <concepts>
#include <condition_variable>

#include <mutex>
#include <queue>
#include <thread>

namespace {
    constexpr std::size_t MAX_THREADS = 32;
}

template <typename FTask,  std::size_t N>
requires (N <= MAX_THREADS)
class ThreadPool {
    std::array<std::jthread, N> threads_;
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
        std::cout << "~ThreadPool()" << std::endl; 
    }
    void Run() {
        for (auto& th : threads_) {
            th = std::jthread([this](std::stop_token st) {
                this->worker(st);
            });
        }
    }

    template <typename... Args>
    requires std::invocable<FTask, Args...>
    void AddTask(FTask func/*, Args&&... args*/) {
        std::scoped_lock lock(tasks_mutex_);
        tasks_.push(func);
    }

private:
    void worker(std::stop_token stoken) {
        // while (is_should_work_) {
            FTask current_task;
            {
                std::unique_lock lock(tasks_mutex_);
                // enable_shared_from_this
                cv_.wait(lock, [this, &stoken] { 
                    return stoken.stop_requested() || !this->tasks_.empty(); 
                });

                if (stoken.stop_requested() && tasks_.empty()) {
                    return;
                }

                current_task = tasks_.back();
                tasks_.pop();
                std::cout << tasks_.size() << std::endl;
            }

            current_task();
            
        //}
    }
};


void foo() {
    std::cout << "My first job!" << std::endl;
}

int main() {
    ThreadPool<void (*)(), 4> pool;
    pool.AddTask(foo);
    pool.AddTask(foo);

    pool.Run();
    // launch in separate thread
    std::cout << "Thread pool!!!" << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(2));
}