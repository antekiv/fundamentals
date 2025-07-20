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


int main()
{
    const Example e{""};
    e.get();
    //e.get();

}