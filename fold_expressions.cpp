#include <iostream>

// 1. fold expressions + comma (since c++17)
template <typename... Args>
void print_all1(Args&&... args) {
    ((std::cout << args << " "), ...);
}


// 2. auto instead of templates (since c++20)
void print_all2(auto&&... args) {
    ((std::cout << args << " "), ...);
}


// 3. old-style overloads
void print_all3() {}

template<typename Head>
void print_all3(Head&& h) {
    std::cout << h << std::endl;
}

template<typename Head, typename... Tail>
void print_all3(Head&& h, Tail&&... t) {
    std::cout << h << " ";
    print_all3(std::forward<Tail>(t)...);
}


// 4. old-style template specializations
template<typename... Args>
void print_all4(Args&&... args);

template<>
void print_all4<>() {
    std::cout << "\n";
}

template<typename Head, typename... Tail>
void print_all4(Head&& h, Tail&&... t) {
    std::cout << h << " ";
    print_all4(std::forward<Tail>(t)...);
}

int main() {
    print_all4(43, 4.2323, false, "Hello foldsdfsdfdsf !");
    print_all4();
}