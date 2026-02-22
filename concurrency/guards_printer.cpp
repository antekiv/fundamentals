#include <iostream>
#include <format>
#include <mutex>
#include <thread>
#include <vector>

class Printer {
    std::mutex m_mtx;

public:
    Printer() = default;
    void Print(const std::string& message) {
        std::unique_lock lock{m_mtx, std::try_to_lock};

        while (!lock.owns_lock()) {
            std::cout << std::format("id: {}, Ok. It's busy. I go to drink coffe", std::hash<std::thread::id>{}(std::this_thread::get_id())) << std::endl;
            std::this_thread::sleep_for(std::chrono::microseconds(200));
            lock.try_lock();
        }

        std::cout << std::format("id: {}, message: {}", std::hash<std::thread::id>{}(std::this_thread::get_id()), message) << std::endl;
        std::this_thread::sleep_for(std::chrono::microseconds(100));
    }
};

int main() {

    Printer printer;
    std::vector<std::thread> threads;
    for (int i = 0; i < 5; ++i) {
        threads.emplace_back(&Printer::Print, &printer, std::string("I cautch the printer!"));
    }

    for (auto& thread : threads)
        thread.join();
}