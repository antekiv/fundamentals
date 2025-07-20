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


int main()
{
    std::vector v = {1, 2, 3};
    vector v2(v.begin(), v.end());
}  