// Class Template Argument Deduction

// C++17
// std::vector v = {1, 2, 3};
// std::vector v2 = {v.begin(), v.end()}; - vector with 2 iterators

#include <vector>

template <typename T>
struct vector {
    template <typename Iter>
    vector(Iter, Iter) {}
};
// Explicit template deduction guide
template <typename Iter>
vector (Iter, Iter)
    -> vector<typename std::iterator_traits<Iter>::value_type>;


#include <iostream>

struct S {
    int i;
};

/*
std::vector v = {1, 2, 3};
vector v2(v.begin(), v.end());
*/
  


// get for rvalue, const
// SFINAE (Substitution Failure Is Not An Error)
// Works only with declaration of the function. SFINAE in defenition is CE
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



#include <type_traits>
#include <string>

struct ImplicitBox {
    ImplicitBox(int) {} 
    operator int() const { return {}; }
};

struct ExplicitBox {
    explicit ExplicitBox(int) {}
    explicit operator int() const { return {}; }
};

int main() {
    //std::cout << std::is_convertible_v<int, ImplicitBox> << std::endl;
    //std::cout << std::is_convertible_v<ImplicitBox, int> << std::endl;

    // means int i(ImplicitBox);
    // means ImplicitBox ib(i);
    //std::cout << std::is_constructible_v<int, ImplicitBox> << std::endl;
    //std::cout << std::is_constructible_v<ImplicitBox, int> << std::endl;

    // Explicit
    // means int i = ExplicitBox();
    // means ExplicitBox eb = 1; 
    //std::cout << std::is_convertible_v<int, ExplicitBox> << std::endl;
    //std::cout << std::is_convertible_v<ExplicitBox, int> << std::endl;

    //ImplicitBox eb = 1; 

    
    //std::cout << std::is_constructible_v<int, ImplicitBox> << std::endl;
    //std::cout << std::is_constructible_v<ImplicitBox, int> << std::endl;
}