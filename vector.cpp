#include <iostream>
#include <type_traits>

// cppreference requirements: Container, Iterator !!!!
// cppinsights.io
//
// iterator_traits:
// std::is_base_of()`

// std::prev(), std::next();


// PoolAllocator - allocate a big array, and then give small peaces
// StackAllocator

// container -> allocator_traits -> allocator -> operator new -> malloc -> OS
template <typename T>
struct allocator {
    T* allocate(size_t count) {
        // execute the first part of operator new (return count * sizeof(T) bytes)
        return operator new(count * sizeof(T));
    }
    void deallocate(T* ptr, size_t) {
        // execute the second part of operator delete (deallocate n bytes [count_of_bytes][[*ptr] buffer]) 
        operator delete(ptr);
    }

    template <typename U, typename... Args>
    void construct(U* ptr, const Args&&... args) {
        new (ptr) U(std::forward(args)...);
    }

    // for example list
    template <typename U>
    void destroy(U* ptr) {
        ptr->~U();
    }

    template <typename U>
    allocator(allocator<U>){}

    template <typename U>
    struct rebind {
        using other = allocator<U>;
    };
};

template <typename T, typename Alloc = std::allocator<T>>
class vector {
    T*      arr_ = nullptr;
    size_t   sz_ = 0;
    size_t  cap_ = 0;
    Alloc alloc_ = Alloc();

private:
    template <bool IsConst>
    class base_iterator {
    public:
        using pointer_type = std::conditional_t<IsConst, const T*, T*>;
        using reference_type = std::conditional_t<IsConst, const T&, T&>;
        using value_type = T;
    private:
        pointer_type ptr_;
        
    public:
        base_iterator(T* ptr): ptr_(ptr) {}
        base_iterator(const base_iterator&) = default;
        base_iterator& operator=(const base_iterator&) = default;

        bool operator==(const base_iterator& other) const {
            return ptr_ == other.ptr_;
        }
        
        reference_type operator*() const {return *ptr_;};
        pointer_type operator->() const {return ptr_;}

        base_iterator& operator++() {
            ++ptr_;
            return *this;
        }
        base_iterator& operator++(int) {
            base_iterator copy = *this;
            ++ptr_;
            return copy;
        }
    };

public:
    using const_iterator = base_iterator<true>;
    using iterator = base_iterator<false>;
    
    iterator begin() {
        return {arr_};
    }

    iterator end() {
        // decrement to end() should get the last element
        return {arr_ + sz_};
    }

    const_iterator begin() const {
        return {arr_};
    }

    const_iterator end() const {
        return {arr_ + sz_};
    }

    const_iterator cbegin() const {
        return {arr_};
    }

    const_iterator cend() const {
        return {arr_ + sz_};
    }

public:
    using AllocTraits = std::allocator_traits<Alloc>;
    explicit vector(size_t count = 0, const Alloc& alloc = Alloc())
        : sz_(count)
        , alloc_(alloc) {
            reserve(count);
        } 

    ~vector() {
        for (size_t i = 0; i < sz_; ++i) {
            AllocTraits::destroy(alloc_, arr_ + i);
        }

        AllocTraits::deallocate(alloc_, arr_, cap_);
    }
    // auto since C++20 
    void emplace_back(auto&&... args) {
        if (sz_ == cap_) {
            reserve(cap_ > 0 ? cap_ * 2 : 1);
        }

        AllocTraits::construct(alloc_, arr_ + sz_, std::forward<decltype(args)>(args)...);
        ++sz_;
    }

    template <typename... Args>
    void push_back(Args&&... args) {
        emplace_back((args)...);
    }

    vector& operator=(const vector& other) const &
    {
        Alloc new_alloc = AllocTraits::propagate_on_container_copy_assigment::value
            ? other.alloc_ : alloc_;

        T* new_arr = AllocTraits::allocate(new_alloc, other.sz_);
        size_t copied = 0;
        try { 
            for (; copied < sz_; ++copied) { 
                AllocTraits::construct(new_alloc, new_arr + copied, arr_[copied]);
            }
        } catch (...) {
            for (size_t i = 0; i < copied; ++i) {
                AllocTraits::destroy(new_alloc, new_arr + i);
            }

            AllocTraits::deallocate(new_alloc, new_arr, other.sz_);
            throw;
        }
 
        for (size_t i = 0; i < sz_; ++i) {
            AllocTraits::destroy(alloc_, arr_ + i);
        }
        AllocTraits::deallocate(alloc_, arr_, cap_);

        alloc_ = new_alloc; // not throw 
        arr_ = new_arr;
        sz_ = other.sz_;
        cap_ = other.cap_;
    }

    size_t capacity() const {
        return cap_;
    }

    size_t size() const {
        return sz_;
    }

    void reserve(size_t new_cap) {
        if (new_cap <= cap_)
            return;
    
        // T may not a constructor by default
        // BAD: T* new_arr = new T[new_cap];
        // exception safety (ES): Ok if new throw an exception. All objects are valid
        // T* new_arr = reinterpret_cast<T*>(new char[new_cap * sizeof(T)]);
        T* new_arr = AllocTraits::allocate(alloc_, new_cap);
        size_t copied = 0;
        try { 
            // UB explicit cast to T
            // for (size_t i = 0; i < sz_; ++i) {
            //     new_arr[i] = arr_[i];
            // }
            // memcpy won't work if field points to another field (see struct Strange)

            // need to execute a ctor on raw memory
            // ES: T can throw an exception. It's bad. Need to delete all new objects
            for (; copied < sz_; ++copied) {
                // new (new_arr + current_copied) T(arr_[current_copied]); 
                // move-ctor should be noexcept 
                AllocTraits::construct(alloc_, new_arr + copied, std::move_if_noexcept([copied]));
            }

            // new (new_arr + sz) T(args)
            // std::is_trivially_copyable 
        } catch (...) {
            for (size_t old_i = 0; old_i < copied; ++old_i) {
                AllocTraits::destroy(alloc_, new_arr + old_i);
            }

            AllocTraits::deallocate(alloc_, new_arr, new_cap);
            throw;
        }
 
        // remove old objects
        for (size_t i = 0; i < sz_; ++i) {
            AllocTraits::destroy(alloc_, arr_ + i);
        }
        AllocTraits::deallocate(alloc_, arr_, cap_);

        arr_ = new_arr;
        cap_ = new_cap;
    }
};



// Debug(v[5]); v - vector bool
// vector<bool> is an example when rvalue can and must! be assigned to new value (28)
#include <iostream>

// won't alive if copied by memcpy
struct Strange {
    int x_;
    int& r_;
    Strange(int x) : x_(x), r_(x_) {}
};

struct Trace {};
struct Throw 
{
    Throw() { throw std::runtime_error("Exception while constructing Throw");}
};

struct ThrowInAssigment 
{
    ThrowInAssigment() = default;
    ThrowInAssigment& operator=(const ThrowInAssigment&) {
        throw 1;
        return *this;
    }
};

struct Base {};
struct Derived : Base {};

//using base ptr or\ virtual des-or

// vector<string> v(5, "abc");
// v.push_back(v[3]);
// insert - f
int main()
{
    vector<int> v;
    v.push_back(1);
    v.push_back(2);
    v.push_back(3);
    v.push_back(3);
    v.push_back(4);

    for (const auto& e : v)
    {
        std::cout << e;
    }

    std::cout << "\nsize: " << v.size();
    std::cout << "\ncap: " << v.capacity(); 
}