#include <iostream>
#include <thread>

// Types of executing of new thread 

// 1. launch a function
void print_hello() {
    std::cout << "Hello World!" << std::endl;
}

// 2. launch a functional object
class BackgroundTask {
public:
    void operator()() const {
        std::cout << "Hello World from functional object!" << std::endl;
    }
};

// 3. launch from lambda
auto lambda = []() {
    std::cout << "Hello World from lambda!" << std::endl;
};

int main() {
    // 1
    std::thread t1(print_hello);

    // 2 - copies in thread storage 
    BackgroundTask bt;
    std::thread t2(bt);

    // 3
    std::thread t3(lambda);

    t1.join();
    t2.join();
    t3.join();
}