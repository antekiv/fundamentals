#include <iostream>

// Template type deduction
template<typename T>
T&& move(T& x) {
    return static_cast<T&&>(x);
}

int main() {
    int i = 10;
    int&& k = move(i);
    
    ++k;
    k += 54;

    int& y = static_cast<int&>(i);

    y = 4;
    std::cout << i << std::endl;

}