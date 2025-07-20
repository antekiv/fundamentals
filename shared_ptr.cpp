#include <iostream>
#include <memory>

template <typename T, typename Deleter = std::default_delete<T>>
class shared_ptr {

    // Two ways to construct:
    // 1. ctor:
    //    T* -> T
    //    ctrlBlock_* -> ControlBlock
    //
    // 2. make_shared:
    //    T* -> T
    //    ctrlBlock_* -> nullptr
    //    it means that counters lay before T (ControlBlockWithObject)
    T* ptr_;
    ControlBlock* ctrl_block_;

    struct ControlBlock {
        size_t shared_count_;
        size_t weak_count_;
    };

    // should be U because of T may not
    // to point at the beggining of the object 
    // std::static_pointer_cast
    struct ControlBlockWithObject : ControlBlock{
        T value_;
    };

    template <typename T, typename... Args>
    friend shared_ptr<T> make_shared(Args&&...);
    
    // ToDo
    shared_ptr(ControlBlock* cp);


public:
    shared_ptr(T* ptr)
            : ptr_(ptr), ctrl_block_(new ControlBlock(1, 0)) {
        
        if constexpr (std::is_base_of_v<T, enable_shared_from_this<T>>) {
            ptr_->sptr_ = *this;
        }
    }
    // TODO: create a shared_ptr from another type
    // auto p = make_shared<Derived>();
    // shared_ptr<Base> bp = p;


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
    weak_ptr<T> sptr_;

    enable_shared_from_this() {}
    shared_ptr<T> shared_from_this() const {
        return sptr_.lock();
    }

    template <typename T>
    friend class shared_ptr;

};



int main() {
    std::cout << "Hello Shared World!" << std::endl;

    std::shared_ptr<int> p = std::make_shared<int>(5);
    std::weak_ptr<int> pw = p;
}