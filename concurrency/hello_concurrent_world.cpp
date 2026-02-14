#include <iostream>
#include <thread>

void print_hello() {
    std::cout << "Hello World!" << std::endl;
}

int main() {
    std::thread t(print_hello);
    t.join();
}