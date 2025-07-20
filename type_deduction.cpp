// c++11
// trailing return type
/*
template <typename Cont>
auto getVal(Cont c, int i) -> decltype(c[i])
{
    return c[i];
}
*/

/* The same as getVal (C++14)
    template <typename Cont>
    decltype(auto) getVal(Cont c, int i)
    {
        return c[i];
    }
*/


// auto&& x is same as T&& - universe reference 
// decltype(auto) - auto using decltype rules

// deducing this
// C++23
// or auto decl

// auto& 
// auto&& - lvalue or rvalue
// stdd::forward_like

#include <iostream>

template <class T, class U>
constexpr decltype(auto) forward_like(U&& x) noexcept {
    if constexpr (std::is_lvalue_reference_v<T>) {
        if constexpr (std::is_const_v<std::remove_reference_t<T>>)
            return static_cast<const std::remove_reference_t<U>&>(x);
        else
            return static_cast<std::remove_reference_t<U>&>(x);
    } else {
        if constexpr (std::is_const_v<std::remove_reference_t<T>>)
            return static_cast<const std::remove_reference_t<U>&&>(std::forward<U>(x));
        else
            return static_cast<std::remove_reference_t<U>&&>(std::forward<U>(x));
    }
}

struct Example {
    std::string value_;

    decltype(auto) get(this auto&& self) {
        return forward_like<decltype(self)>(self.value_);
    }
};


// METAAAA
template <typename T>
struct type_identity {
    using type = T;
};

template <typename T>
using type_identity_t = type_identity<T>::type;


// integral_constant
template <typename T, T v>
struct integral_constant {
    static constexpr T value = v;
};

template <bool b>
using bool_constant = integral_constant<bool, b>;

using true_type = bool_constant<true>;
using false_type = bool_constant<false>;



template <typename... Types>
struct conjunction {
    static constexpr bool value = (Types::value && ...);
};

template <typename... Types>
constexpr bool conjunction_v = conjunction<Types...>::value;


// SFINAE = Substitution Failture Is Not An Error!
// works only for declarations!!!


template <bool B, typename T = void>
struct enable_if {};

template <typename T>
struct enable_if<true, T> {
    using type = T;
};

template <bool B, typename T = void>
using enable_if_t = enable_if<B, T>::type;


template <typename T>
enable_if_t<std::is_integral_v<T>, void> foo(T)
{
    std::cout << "Integral!\n";
}

template <typename T>
enable_if_t<!std::is_integral_v<T>, void>  foo(T)
{
    std::cout << "Other!\n";
}

// Is class has some method

namespace detail {
    template <typename T, typename... Args>
    std::true_type test( decltype(T().construct(Args()...))* );

    std::false_type test(...);
}
template <typename T, typename... Args>
struct has_method_construct : decltype(detail::test<T, Args...>(nullptr)) {};



// Constraits and requirements (since c++20)
template<typename T>
requires std::is_integral_v<T> // && ... 
void foo1(T) {
    std::cout << "Integral!\n";
}

template<typename T>
requires std::negation_v<std::is_integral<T>> // && ... 
void foo1(T) {
    std::cout << "Other!\n";
}


int main()
{
    foo1(5);
    foo1("fwefwef");
}