#include "string.h"
#include <cassert>
#include <iostream>

int main() {

    string s;
    assert(s.size() == 0);
    assert(s.c_str() == nullptr);

    // append
    {
        string s1("hi");
        string s2("hello");
        string s3("hello");

        std::cout << s3 << std::endl;
    }
    {
        // the second string won't be changed
        // string s = "Anton, "_s;
        const string s0 = "Hello World!"_s;

        s.append(s0);

        assert(std::strcmp(s.c_str(), "Anton, Hello World!") == 0);
        assert(s.size() == 19);

        std::cout << s << std::endl;
    }

    //assert(strcmp(("Hello, world!"_s).c_str(), ""));

    string s1(10, 'c');
    assert(s1.size() == 10);
    assert(std::strcmp(s1.c_str(), "cccccccccc") == 0);

    // throws std::bad_alloc
    // string s2(-4, '3');

    // add checking
    // operator[]
    {

    }


    std::cout << s << std::endl;

}
