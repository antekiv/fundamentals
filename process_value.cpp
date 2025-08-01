#include <iostream>
#include <type_traits>

template <typename T>
auto process_value(T&& value) {
    if constexpr (std::is_integral_v<std::remove_reference_t<T>>)
        return value * value;
    else if constexpr (std::is_floating_point_v<std::remove_reference_t<T>>)
        return value / 2;
    else if constexpr (std::is_convertible_v<std::remove_reference_t<T>, std::string>)
        return value.size();
    else
        static_assert(false, "Incorrect type!");
}
/*
    int i = 5;
    std::cout << (process_value(i)) << std::endl;
    double d = 54.343;
    std::cout << (process_value(d)) << std::endl;
    auto s = std::string("Hello World!");
    std::cout << (process_value(s)) << std::endl;
    process_value(S());
*/

/*
template <typename T>
requires std::is_integral_v<std::remove_reference_t<T>>
     ||  std::is_floating_point_v<std::remove_reference_t<T>>
T log_and_forward(T& value) {
    std::cout << typeid(T).name() << " lvalue" << std::endl;
    return value;
}

template <typename T>
requires std::is_integral_v<std::remove_reference_t<T>>
     ||  std::is_floating_point_v<std::remove_reference_t<T>>
T&& log_and_forward(T&& value) {
    std::cout << typeid(T).name() << " rvalue" << std::endl;
    return std::forward<T>(value);
}

const int i = 34;
const int& ir = i;
log_and_forward(ir);
*/

// Sum<5>::value = 5 + 4 + ... + 0
template <size_t N>
struct Sum {
    static constexpr size_t value = N + Sum<N-1>::value;
};

template <>
struct Sum<0> {
    static constexpr size_t value = 0;
};


// Factorial
template <size_t N>
struct Factorial {
    static constexpr size_t value = N * Factorial<N-1>::value;
};

template <>
struct Factorial<1> {
    static constexpr size_t value = 1;
};


// Fibonacci
template <size_t N>
struct Fibonacci {
    static constexpr size_t value = Fibonacci<N-1>::value + Fibonacci<N-2>::value;
};

template <>
struct Fibonacci<1> {
    static constexpr size_t value = 1;
};

template <>
struct Fibonacci<0> {
    static constexpr size_t value = 0;
};


// CountType
template <typename T, typename Head, typename ...Tail>
struct CountType {
    static constexpr T value = std::is_same_v<T, Head>::value + CountType<T, Tail>;
};

template <typename T, typename Head>
struct CountType {
    static constexpr T value = std::is_same_v<T, Head>::value;
};

int main() {
    std::cout << Fibonacci<12>::value << std::endl;
}