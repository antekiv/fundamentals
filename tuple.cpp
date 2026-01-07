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

template <>
struct Tuple<> {};

template <typename Head, typename... Tail>
class Tuple<Head, Tail...> {
    Head head_;
    [[no_unique_address]] Tuple<Tail...> tail_;

    template <size_t, typename... Types>
    constexpr friend decltype(auto) get(Tuple<Types...>&);
    template <size_t, typename... Types>
    constexpr friend decltype(auto) get(const Tuple<Types...>&);
    template <size_t, typename... Types>
    constexpr friend decltype(auto) get(Tuple<Types...>&&);
    template <typename, typename... Types>
    constexpr friend decltype(auto) get(Tuple<Types...>& t);

    template <typename...>
    friend struct Tuple;
public:

    Tuple() = default;
    Tuple(const Tuple& other) = default;

    Tuple(Tuple&& other)
        : head_(std::forward<Head>(other.head_))
        , tail_(std::move(other.tail_)) 
    {}
    
    explicit (
        ! (std::is_convertible_v<const Head&, Head> &&
          (std::is_convertible_v<const Tail&, Tail> && ...))
    )
    Tuple(const Head& head, const Tail&... tail)
        requires (
             std::is_copy_constructible_v<Head> &&
            (std::is_copy_constructible_v<Tail> && ...))
        : head_(head)
        , tail_(tail...) 
    {}

    template <typename UHead, typename... UTail>
    requires (
        sizeof...(UTail) == sizeof...(Tail) &&
        std::is_constructible_v<Head, UHead> &&
       (std::is_constructible_v<Tail, UTail> && ...)
    )
    explicit ( 
        ! (std::is_convertible_v<UHead, Head> && 
          (std::is_convertible_v<UTail, Tail> && ...))
    )
    Tuple(UHead&& head, UTail&&... tail)
        : head_(std::forward<UHead>(head))
        , tail_(std::forward<UTail>(tail)...)
    {}


    template <typename UHead, typename... UTail>
    requires (
        sizeof...(Tail) == sizeof...(UTail) &&
        std::is_constructible_v<Head, UHead> &&
       (std::is_constructible_v<Tail, UTail> && ...)
    )
    explicit ( 
        ! (std::is_convertible_v<UHead, Head> && 
          (std::is_convertible_v<UTail, Tail> && ...))
    )
    Tuple(const Tuple<UHead, UTail...>& other)
        : head_(other.head_)
        , tail_(other.tail_) 
    {}

    template <typename UHead, typename... UTail>
    requires (
        sizeof...(Tail) == sizeof...(UTail) &&
        std::is_constructible_v<Head, UHead> &&
       (std::is_constructible_v<Tail, UTail> && ...)
    )
    explicit( 
        ! (std::is_convertible_v<UHead, Head> && 
          (std::is_convertible_v<UTail, Tail> && ...))
    )
    Tuple(Tuple<UHead, UTail...>&& other)
        : head_(std::forward<UHead>(other.head_))
        , tail_(std::move(other.tail_)) 
    {}

    template <typename UHead, typename... UTail>
    requires (
        sizeof...(Tail) == sizeof...(UTail) &&
        std::is_assignable_v<Head&, const UHead&> &&
    (std::is_assignable_v<Tail&, const UTail&> && ...)
    )
    Tuple& operator=(const Tuple<UHead, UTail...>& other) {
        head_ = other.head_;
        tail_ = other.tail_;
        return *this;
    }

    template <typename UHead, typename... UTail>
    requires (
         sizeof...(Tail) == sizeof...(UTail) &&
         std::is_assignable_v<Head&, UHead&&> &&
        (std::is_assignable_v<Tail&, UTail&&> && ...)
    )
    Tuple& operator=(Tuple<UHead, UTail...>&& other) {
        head_ = std::forward<UHead>(other.head_);
        tail_ = std::move(other.tail_);
        return *this;
    }

    Tuple& operator=(const Tuple& other)
    requires (
         std::is_copy_assignable_v<Head> &&
        (std::is_copy_assignable_v<Tail> && ...)
    ) {
        head_ = other.head_;
        tail_ = other.tail_;

        return *this;
    }
    Tuple& operator=(Tuple&& other) 
    requires (
         std::is_move_assignable_v<Head> &&
        (std::is_move_assignable_v<Tail> && ...)
    ) {
        head_ = std::move(other.head_);
        tail_ = std::move(other.tail_);

        return *this;
    }
};

template <size_t N, typename... Types>
constexpr decltype(auto) get(const Tuple<Types...>& t) {
    if constexpr (N == 0) {
        return (t.head_); 
    } else {
        return get<N - 1>(t.tail_);
    }
}

template <size_t N, typename... Types>
constexpr decltype(auto) get(Tuple<Types...>& t) {
    if constexpr (N == 0) {
        // t.head_ — is an access to the field. decltype(t.head_) returns type of the field (for example T).
        // (t.head_) — is expression. decltype((t.head_)) for lvalue returns reference (T&).
        using FieldType = decltype(t.head_);
        return static_cast<FieldType&>(t.head_);
    } else {
        return get<N - 1>(t.tail_);
    }
}

template <size_t N, typename... Args>
constexpr decltype(auto) get(Tuple<Args...>&& t) {
    if constexpr (N == 0) {
        return std::move(t.head_);
    } else {
        return get<N - 1>(std::move(t.tail_));
    }
}

template <typename T, typename... Types>
constexpr decltype(auto) get(Tuple<Types...>& t) {
    if constexpr (std::is_same_v<T, decltype(t.head_)>) {
        return (t.head_); 
    } else {
        return get<T>(t.tail_);
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