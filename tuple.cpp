#include <iostream>
#include <cassert>
#include <type_traits>
#include <vector>

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
struct Tuple;

template <typename Head, typename... Tail>
class Tuple<Head, Tail...> {
    Head head_;
    [[no_unique_address]] Tuple<Tail...> tail_;

    // and 3 overloads :thinking:
    template <size_t N, typename... Types>
    friend decltype(auto) get(Tuple<Types...>&);
public:

    Tuple(const Head& head, const Tail&... tail)
        : head_(head)
        , tail_(tail...) 
    {}
    template <typename UHead, typename... UTail>
    requires (
        sizeof...(UTail) == sizeof...(Tail) &&
        std::is_constructible_v<Head, UHead> &&
        (std::is_constructible_v<Tail, UTail> && ...)
    )
    explicit( 
        ! (std::is_convertible_v<UHead, Head> && 
          (std::is_convertible_v<UTail, Tail> && ...))
    )
    Tuple(UHead&& head, UTail&&... tail)
        : head_(std::forward<UHead>(head))
        , tail_(std::forward<UTail>(tail)...)
    {}

    template <typename UHead, typename... UTail>
    explicit( 
        ! (std::is_convertible_v<UHead, Head> && 
          (std::is_convertible_v<UTail, Tail> && ...))
    )
    Tuple(const Tuple<UHead, UTail...>& other)
        : head_(other.head)
        , tail_(other.tail) 
    {}
};

template <>
struct Tuple<> {};

template <size_t N, typename... Types>
decltype(auto) get(Tuple<Types...>& t) {
    if constexpr (N == 0) {
        return t.head_;
    } else {
        return get<N-1>(t.tail_);
    }
}

template <typename... Types>
struct tuple_size;

template<class... Types >
struct tuple_size<Tuple<Types...>>
    : std::integral_constant<std::size_t, sizeof...(Types)> { };

template<class T>
constexpr size_t tuple_size_v = tuple_size<T>::value;


template <size_t I, typename Tuple>
struct tuple_element {
    using type = decltype(get<I>(std::declval<Tuple&>()));
};