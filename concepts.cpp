#include <iostream>
#include <concepts>
#include <type_traits>
#include <vector> 

// Types of requires

// 1. simple require - checks bool expression
// require a == b || std::is_arifmetic..

// 2. nested require - checks only compilation!!!
// require require (T a) {
//      a++;
//      a == a;

// 3. Type requiment
//      typename T::value_type;
// }


// concepts (since c++20)
struct MyInt {
    int i;

    explicit operator double() const {
        return static_cast<double>(i);
    }
};

int operator+(MyInt lhs, MyInt rhs) {
    return lhs.i + rhs.i;
}

double operator+(double lhs, MyInt rhs) {
    return lhs + rhs.i;
}

template<typename T>
concept Summable = std::is_arithmetic_v<std::remove_reference_t<T>>
                 || requires(T t) {
                        t + t;
                    };

template <Summable... Args>
auto sum(Args&&... args) {
    return (args + ...); 
}

/*
MyInt mi(6);
int i = 65;
std::cout << sum(34, 56.23, 65, -345.34, i, mi);
*/


// Perfect string forwarding
struct StringArray {
    std::vector<std::string> array;

    template <std::convertible_to<std::string>... Strings>
    StringArray(Strings&&... str) {
        array.reserve(sizeof...(str));
        std::cout << "capacity: " << array.capacity() << std::endl;
        (array.emplace_back(std::forward<Strings>(str)), ...);
    }
};

struct Trace {
    Trace() { std::cout << "Default constructor\n"; }
    Trace(const Trace&) { std::cout << "Copy constructor\n"; }
    Trace(Trace&&) noexcept { std::cout << "Move constructor\n"; }
    Trace& operator=(const Trace&) {
        std::cout << "Copy assignment\n";
        return *this;
    }

    Trace& operator=(Trace&&) noexcept {
        std::cout << "Move assignment\n";
        return *this;
    }
    ~Trace( ) {std::cout << "Destructor\n";};
};

struct TraceArray {
    std::vector<Trace> array;

    template <typename... Args>
    TraceArray(Args&&... str) {
        array.reserve(sizeof...(str));
        std::cout << "capacity: " << array.capacity() << std::endl;
        (array.emplace_back(std::forward<Args>(str)), ...);
    }
};



int main() {
    Trace tr;

    TraceArray arr(tr, Trace(), std::move(tr));
}