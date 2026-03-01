#include <condition_variable>
#include <iostream>
#include <queue>

std::mutex mut;
std::queue<std::string> data_queue;
std::condition_variable data_cond;

void data_preparation_thread() {
    const std::string data = "Hello World";
    {
        std::lock_guard<std::mutex> lk(mut);
        std::cout << "data_preparation_thread" << std::endl; 
        data_queue.push(data);
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }
    data_cond.notify_one();
}

void data_processing_thread() {
    while (true) {
        std::unique_lock<std::mutex> lk(mut);
        data_cond.wait(
            lk, []{
                std::cout << "I woked!" << std::endl;
                return !data_queue.empty();}
        );

        std::string data = data_queue.front();
        data_queue.pop();

        std::cout << "data_processing_thread: " << data << std::endl;
    }
}

int main() {
    std::thread t1{data_processing_thread};
    std::thread t2{data_preparation_thread};

    t1.join();
    t2.join();
}


