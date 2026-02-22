#include <iostream>
#include <mutex>
#include <vector>
#include <thread>

struct Account {
    std::string m_name;
    int m_money;
    std::mutex m_mutex;
};

void transfer(Account& from, Account& to, int amount) {
    /*
    // An old style until C++14
    std::unique_lock<std::mutex> lock1(from.m_mutex, std::defer_lock);
    std::unique_lock<std::mutex> lock2(to.m_mutex, std::defer_lock);
    // ... here can execute code which doesn't need to block ...
    // 2. Safely block both mutexes (algo avoids deadlocks)
    std::lock(lock1, lock2);
    */

    // Modern way since C++17
    std::scoped_lock lock(from.m_mutex, to.m_mutex);
    std::cout << "Try to transfer: " << from.m_name << " -> " << to.m_name << std::endl;
    from.m_money -= amount;
    to.m_money   += amount;
    std::cout << "result: " << from.m_money << " -- " << to.m_money << std::endl;
}

void do_transfer_1000(Account& from, Account& to, int amount) {
    for (int i = 0; i < 1000; ++i) {
        transfer(from, to, amount);
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
}

int main() {
    Account bob = {"Bob", 1000};
    Account alice = {"Alice", 1000};
    Account ren = {"Ren", 1000};

    std::thread t1{do_transfer_1000, std::ref(bob), std::ref(alice), 10};
    std::thread t2{do_transfer_1000, std::ref(alice), std::ref(ren), 10};
    std::thread t3{do_transfer_1000, std::ref(ren), std::ref(bob), 10};

    t1.join();
    t2.join();
    t3.join();

    std::cout << "bob: " << bob.m_money << std::endl;
    std::cout << "alice: " << alice.m_money << std::endl;
    std::cout << "ren: " << ren.m_money << std::endl;
}
