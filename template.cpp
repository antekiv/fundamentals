#include <iostream>
#include <vector>

// 1. Template - any class
// 2. Template - non-type - (bool, char, size_t, ...)
// 3. Template Template
 
// NTTP - non-type template parameters

template <typename T, size_t N>
class array {
    T arr[N];
};

template <bool B>
struct BoolType {};

template <>
struct BoolType<true> {
    static constexpr bool value = true;
};

template <>
struct BoolType<false> {
    static constexpr bool value = false;
};

// is not a structural type because it has a non-static data member that is not public
// Alloc_hider      _M_dataplus;
// template <std::string I>
// struct String {};

//
// Template Template parameters
//

template <typename T, 
          template <typename, typename>
          class Container = std::vector> // need to write class until C++17 (after - class/typename)
class Stack {
    Container<T, std::allocator<T>> container;
};

int main() {
    std::cout << "true type: " << BoolType<true>::value << std::endl;
    std::cout << "false type: " << BoolType<false>::value << std::endl;
    
    std::cout << "Hello Template World!\n" << std::endl;

    Stack<int, std::vector> s;
}