#include <iostream>
#include <type_traits>

namespace {
    template <typename T>
    struct is_lvalue_reference : std::false_type {};

    template <typename T>
    struct is_lvalue_reference<T&> : std::true_type {};


    template <typename T>
    struct is_rvalue_reference : std::false_type {};

    template <typename T>
    struct is_rvalue_reference<T&&> : std::true_type {};


    template <typename T>
    struct is_reference : std::false_type {};

    template <typename T>
    struct is_reference<T&> : std::true_type {};

    template <typename T>
    struct is_reference<T&&> : std::true_type {};

}

template <typename T>
inline constexpr bool is_lvalue_reference_v = is_lvalue_reference<T>::value;

template <typename T>
inline constexpr bool is_rvalue_reference_v = is_rvalue_reference<T>::value;

template <typename T>
inline constexpr bool is_reference_v = is_reference<T>::value;


template <typename T>
void wrapper(T&&)
{
    std::cout << "Lvalue ref: " << is_lvalue_reference_v<T> << std::endl; 
    std::cout << "Rvalue ref: " << is_rvalue_reference_v<T> << std::endl;
    std::cout << "       ref: " << is_reference_v<T>        << std::endl;
}

/*
    Trait	                	                       (int,   int&,  int&&)
    std::is_lvalue_reference_v<T>	is T lvalue-ref?	false, true,  false
    std::is_rvalue_reference_v<T>	is T rvalue-ref?	false, false, true
    std::is_reference_v<T>	        is T any ref?	    false, true,  true

    ference collapsing rules:
    & + &   = &
    & + &&  = &
    && + &  = &
    && + && = &&
*/

template <typename T>
void Debug(T&&) = delete;

void process(int&)  { std::cout << "lvalue\n"; }
void process(int&&) { std::cout << "rvalue\n"; }

template <typename T>
T&& native_forward(T&& value) {
    return value;
}

template<typename T>
void wrapper1(T&& arg) {
    process(native_forward<T>(arg));  // Ваша реализация
    // process(std::forward<T>(arg));
}


// lval 
// int i;
// T = int&
// typeof(args) = T& + T&& = T&
// forward<T&>(int&)
// value = T&
// 

// rval
// 5
// T = int
//
int main() {
    /*
    {
        // or Debug func
        int i = 5;
        int& ri = i;
        ++ri;

        // Forwarding references 
        // lvalue call:  T = int&
        wrapper(i);
        wrapper(ri);
        int&& rvi = 6;
        wrapper(rvi);

        // rvalue call: T = int
        wrapper(6);
        wrapper(std::move(i));

        
    }
    */

    int x = 10;
    wrapper1(x);  // Ваша версия выведет "lvalue" (ОШИБКА!)
                            // Правильный std::forward выведет "rvalue"
}