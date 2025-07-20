#include <iostream>
#include <memory>

template <typename T, typename Deleter = std::default_delete<T>>
class shared_ptr {
    // Two way of construct:
    // 1. Ctor:
    //    T* -> T
    //    ctrlBlock_* -> ControlBlock
    //
    // 2. make_shared:
    //    T* -> T
    //    ctrlBlock_* -> nullptr
    //    it means that counters lay befor T (ControlBlockWithObject)
    T* ptr_;
    ControlBlock* ctrlBlock_;

    struct ControlBlock {
        size_t shared_count_;
        size_t weak_count_;
    };

    struct ControlBlockWithObject : ControlBlock{
        T value_;
    };

    template <typename T, typename... Args>
    friend shared_ptr<T> make_shared(Args&&...);
    
    // ToDo
    shared_ptr(ControlBlock* cp);


public:
    shared_ptr(T* ptr, Deleter del = std::default_delete<T>()) : ptr_(ptr), del_(del) {}
    ~shared_ptr() {
        del_(this->ptr_);
    }
};

template <typename T, typename... Args>
shared_ptr<T> make_shared(Args&&...) {
    auto* p = new shared_ptr<T>::ControlBlock{T(std::forward<Args>(args)...), 1};
    return shared_ptr<T>(p);
}


// weak_ptr, enable_shared_from_this. CRTP

template <typename T>
struct enable_shared_from_this {

    enable_shared_from_this() {}
    shared_ptr<T> shared_froom_this() const; 

};



int main() {
    std::cout << "Hello Shared World!" << std::endl;

    std::shared_ptr<int> p = std::make_shared<int>(5);
    std::weak_ptr<int> pw = p;
}