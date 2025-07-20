#include <array>
#include <chrono>
#include <iostream>
#include <memory>
#include <vector>

// container -> allocator_traits -> allocator -> operator new -> malloc -> OS
template <typename T, size_t N>
struct stack_allocator {
    using value_type = T;

    stack_allocator() noexcept {}

    template <typename U>
    stack_allocator() noexcept {}
    
    T* allocate(size_t) noexcept {
        return reinterpret_cast<T*>(pool_.begin());
    }
    void deallocate(T*, size_t) noexcept {
    }

    template <typename U>
    struct rebind {
        using other = stack_allocator<U, N>;
    };

    bool operator==(const stack_allocator&) const noexcept { return true; }
    bool operator!=(const stack_allocator&) const noexcept { return false; }

private:
    std::array<char, sizeof(T) * N> pool_;
};

const size_t SIZE = 100000;

template <typename Cont>
void bench(Cont& cont)
{
    auto start = std::chrono::high_resolution_clock::now();

    for (size_t i = 0; i < SIZE; ++i)
        cont.push_back(static_cast<int>(i));

    for (auto& e : cont)
        e += 5;

    auto end = std::chrono::high_resolution_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << "time: " << duration.count() << " mcs: " << std::endl;
}

int main()
{
        
    {
        std::vector<int, stack_allocator<int, SIZE>> v_stack;
        v_stack.reserve(SIZE);
        bench(v_stack);
    }
    {
        std::vector<int> v_heap;
        bench(v_heap);
    }
    {
        std::vector<int> v_heap_reserved;
        v_heap_reserved.reserve(SIZE);
        bench(v_heap_reserved);
    }

    
    
    

    std::cout << "Hello Allocator Worlrd!";
}