#include <iostream>
#include <array>
#include <cassert>
#include <concepts>
#include <condition_variable>

#include <functional>
#include <future>

#include <memory>
#include <mutex>
#include <queue>
#include <thread>

// TODO: thread_local query task
namespace {
    constexpr int MAX_THREADS = 32;
}

template <int thread_count>
requires (thread_count > 0 && thread_count <= MAX_THREADS)
class ThreadPool : public std::enable_shared_from_this<ThreadPool<thread_count>> {

    using internal_task_f = std::move_only_function<void()>;
    
    std::queue<internal_task_f> tasks_;
    mutable std::mutex tasks_mutex_;
    std::condition_variable_any cv_;
    std::array<std::jthread, thread_count> threads_;

    std::condition_variable wait_cv_;
    int active_tasks_{0};     
public:
    ThreadPool() = default;
    ~ThreadPool() {
        ForceStop();
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

    void SafeStop() {
        {
            std::unique_lock<std::mutex> lock(tasks_mutex_);
            wait_cv_.wait(lock, [this]() { 
                return tasks_.empty() && active_tasks_ == 0;
            });
        }
        ForceStop();
    }

    void ForceStop() {
        for (auto& th : threads_) {
            th.request_stop();
        }

        for (auto& th : threads_) {
            if (th.joinable()) {
                th.join();
            }
        }
    }

    bool Stopped() const {
        return std::any_of(threads_.cbegin(), threads_.end(), [](const auto& th){ return !th.joinable();} );
    }

    template <typename FTask, typename... Args>
    requires std::invocable<FTask, Args...>
    void AddTask(FTask func/*, Args&&... args*/) {
        {
            std::scoped_lock lock(tasks_mutex_);
            tasks_.push(func);
        }
        cv_.notify_one();
    }

    template <typename FunctionType, typename ...Args>
    requires std::invocable<FunctionType, Args...>
    std::future<typename std::result_of_t<FunctionType(Args...)>>
        Submit(FunctionType f, Args&&... args) {
            using result_type = typename std::invoke_result_t<FunctionType, Args...>;

            std::packaged_task<result_type()> task(
                [f = std::move(f), ...args = std::forward<Args>(args)]() mutable {
                return std::invoke(std::move(f), std::forward<Args>(args)...);
            });
            std::future<result_type> res(task.get_future());

            {
                std::scoped_lock lock(tasks_mutex_);
                tasks_.emplace([ta = std::move(task)]() mutable {
                    ta();
                });
            }
            cv_.notify_one();

            return res;
        }

private:
    void worker(std::stop_token stoken) {

        while (true) {
            internal_task_f current_task;
            {
                std::unique_lock lock(tasks_mutex_);
                // enable_shared_from_this
                cv_.wait(lock, stoken, [this] { 
                    return !this->tasks_.empty(); 
                });

                if (stoken.stop_requested())
                    return;

                if (this->tasks_.empty())
                    continue;

                current_task = std::move(tasks_.front());
                tasks_.pop();
                ++active_tasks_;
            }

            try {
                current_task();
            }
            catch (const std::exception& e) {
                // TODO: print in custom logger
                //std::cout << "[ERROR] while processing task: " << e.what() << std::endl;
            }
            catch (...) {
                //std::cout << "[ERROR] unrecognized throw" << std::endl;
            }

            {
                std::scoped_lock lock(tasks_mutex_);
                --active_tasks_;
                if (tasks_.empty() && active_tasks_ == 0) {
                    wait_cv_.notify_all();
                }
            }
        }
    }
};



namespace test {

template <int T>
constexpr bool explicit_test() {
    return requires {
        std::make_shared<ThreadPool<T>>();
    };
}

int sum(int a, int b) {
    return a + b;
}

void foo() {
    std::cout << "HI!\n";
}

}


int main() {
        
    {
        // construct template test
        static_assert( test::explicit_test<3>());
        static_assert(!test::explicit_test<0>());
        static_assert(!test::explicit_test<-1>());
        static_assert(!test::explicit_test<34>());
    }

    {
        // void func. throw
        std::atomic<int> sum = 0;
        auto incr = [&sum](){
            ++sum;

            if (sum % 100 == 0)
                throw std::runtime_error{"some runtime error"};
         };
        
        auto pool = std::make_shared<ThreadPool<3>>();
        
        for (int i = 0; i < 1'000; ++i)
        {
            pool->AddTask(incr);
        }
        
        assert(pool->Stopped() == true);
        pool->Run();
        assert(pool->Stopped() == false);
        pool->SafeStop();
        assert(pool->Stopped() == true);
        assert(sum == 1'000);
    }

    {
        // 1 producer, 2 consumer
        std::atomic<int> sum = 0;
        auto incr = [&sum](){
            ++sum;
        };

        auto pool = std::make_shared<ThreadPool<2>>();
        pool->Run();

        for (int i = 0; i < 1'000; ++i)
        {
            pool->AddTask(incr);
        }

        pool->SafeStop();
        assert(pool->Stopped() == true);
        assert(sum == 1'000);

        std::cout << "Test: 1 produser, 2 consumer done!" << std::endl;
    }

    {
        // 3 producer, 5 consumer
        std::atomic<int> sum = 0;
        auto incr = [&sum](){
            ++sum;
        };

        auto pool = std::make_shared<ThreadPool<5>>();
        pool->Run();

        auto addTasks = [&](){
            for (int i = 0; i < 10'000; ++i) {
                pool->AddTask(incr);
            }
        };

        std::vector<std::thread> produsers;
        for (int i = 0; i < 3; ++i) {
            produsers.emplace_back(std::thread{addTasks});
        }

        for (auto& th : produsers)
            th.join();

        pool->SafeStop();
        assert(pool->Stopped() == true);
        assert(sum == 30'000);

        std::cout << "Test: 3 produser, 5 consumer done!" << std::endl;
    }
}