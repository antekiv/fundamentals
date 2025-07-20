#include <iostream>
#include <cassert>
#include <type_traits>

// structured bindings c++17
// auto [x, _, _] = ...

// piecewise_construct

// reversed
/*
template <typename Head, typename... Tail>
struct tuple : tuple<Tail...>{
    Head head_;
};
*/

template <typename... Types>
struct tuple;

template <typename Head, typename... Tail>
class tuple<Head, Tail...> {
public:
    Head head_;
    [[no_unique_address]] tuple<Tail...> tail_;

public:
    tuple(Head&& head, Tail&&... tail)
        : head_(head), tail_(std::forward<Tail>(tail)...) {}

    template <size_t N, typename... Types>
    friend decltype(auto) get(tuple<Types...>&);
};

template <>
struct tuple<> {};


template <size_t N, typename... Types>
decltype(auto) get(tuple<Types...>& t) {
    if constexpr (N == 0) {
        return t.head_;
    } else {
        return get<N-1>(t.tail_);
    }
}

template <typename... Types>
struct tuple_size;

template<class... Types >
struct tuple_size<tuple<Types...>>
    : std::integral_constant<std::size_t, sizeof...(Types)> { };

template<class T>
constexpr size_t tuple_size_v = tuple_size<T>::value;


template <size_t I, typename Tuple>
struct tuple_element {
    using type = decltype(get<I>(std::declval<Tuple&>()));
};

// get for rvalue, const

void test_empty() {
    tuple<> t;
    assert(tuple_size_v<decltype(t)> == 0);
}

void test_basic_storage_and_access() {
    tuple<int, std::string, double> t(42, "hello", 3.14);
    assert(get<0>(t) == 42);
    assert(get<1>(t) == "hello");
    assert(get<2>(t) == 3.14);
    assert(tuple_size_v<decltype(t)> == 3);
}

void structure_bindings()
{
    tuple<int, double> t(42, 3.14);
    const auto& [a, b] = t;
    std::cout << a << " -> " << b.head_;
}


// SFINAE (Substitution Failure Is Not An Error)
template<typename T>
typename std::enable_if_t<std::is_integral<T>::value, void>
print_type(T value) {
    std::cout << "Целое число: " << value << std::endl;
}

template<typename T>
typename std::enable_if<std::is_floating_point<T>::value, void>::type
print_type(T value) {
    std::cout << "Число с плавающей точкой: " << value << std::endl;
}


int main()
{
    // test_empty();
    // test_basic_storage_and_access();
    // structure_bindings();

    //print_type(65);
    //print_type(65.545);

}