#include <iostream>
#include <thread>

void print_hello(const std::string& name) {
    std::cout << "Hello, " << name << "!\n";
}

void change_string(std::string& str) {
    str = "Another Words";
}

struct Trace {
    Trace() {std::cout << "def c-tor\n";}
    Trace(const Trace&) {std::cout << "copy c-tor\n";}
    Trace(Trace&&) {std::cout << "move c-tor\n";}
    ~Trace() {std::cout << "d-tor\n";}
};
void trace_move(Trace t) {
    t;
}


int main() {
    {
        /* 1. Copy "as is" (dangling reference)
        char word[] = "World";
        // By default params copy in internal storage assotiated with
        // new thread as is and than transfer to execute thread as rvalue and transform to needed
        // The same expected for references
        // use std::string(word)
        std::thread t(print_hello, word);
        t.detach();
        */
    }

    {
        // 2. Refs are forbitted by default
        // Use wrapper std::ref
        /*
        std::string words = "Hello World!";
        std::thread t(change_string, std::ref(words));
        t.join();
        std::cout << words << std::endl;
        */
    }

    {
        // 3. I like to move it, move it!!!
        Trace trace;

        // 
        std::thread t(trace_move, std::move(trace));
        t.join();
    }

    {
        // 4. class-method
        class X {
            public:
            void do_work() {}
        } my_x;

        std::thread t(&X::do_work, my_x);
        t.join();
    }

    std::this_thread::sleep_for(std::chrono::seconds(1));
}