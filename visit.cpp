#include <iostream>
#include <variant>
#include <vector>

// overload pattern
template <typename... Ts>
struct Overload : Ts... {
	using Ts::operator()...;
};

// deduction guide (no need since c++20)
//template<class... Ts> Overload(Ts...) -> Overload<Ts...>;

Overload ov = {
	[](const int& i) { std::cout << "int: " << i << std::endl;},
    [](const double& d) { std::cout << "double: " << d << std::endl;},
};

/*
  std::vector<std::variant<int, double>> vec {1, 4.34, 423.345, 434};
  
  for (const auto& v : vec)
    std::visit(ov, v);
*/


// Implementation of std::variant and std::visit
template <typename... Types>
union VariadicUnion;

template <typename Head, typename... Tail>
union VariadicUnion<Head, Tail...> {
    Head head;
    VariadicUnion<Tail...> tail;
};

template <typename... Types>
class Variant : private VariantAlternative<Types>... {
private:
    //VariadicUnion<Types...> varun;
    char []
    size_t active_index;

public:
    using VariantAlternative<Types>::VariantAlternative...; 
};

int main()
{
   
}