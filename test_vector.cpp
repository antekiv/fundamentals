#include "vector.h"

#include <iostream>

// won't alive if copied by memcpy
struct Strange {
    int x_;
    int& r_;
    Strange(int x) : x_(x), r_(x_) {}
};

struct Trace {};
struct Throw 
{
    Throw() { throw std::runtime_error("Exception while constructing Throw");}
};

struct ThrowInAssigment 
{
    ThrowInAssigment() = default;
    ThrowInAssigment& operator=(const ThrowInAssigment&) {
        throw 1;
        return *this;
    }
};

struct Base {};
struct Derived : Base {};

//using base ptr or\ virtual des-or

// vector<string> v(5, "abc");
// v.push_back(v[3]);
// insert - f
int main()
{
    vector<int> v;
    v.push_back(1);
    v.push_back(2);
    v.push_back(3);
    v.push_back(3);
    v.push_back(4);

    for (const auto& e : v)
    {
        std::cout << e;
    }

    std::cout << "\nsize: " << v.size();
    std::cout << "\ncap: " << v.capacity(); 
}