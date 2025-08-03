#include <iostream>
#include <memory>

template <typename T>
struct EnableSharedFromThis;

template <typename T, typename Deleter = std::default_delete<T>>
class SharedPtr {

    // Two ways to construct:
    // 1. ctor:
    //    T* -> T
    //    ctrlBlock_* -> ControlBlock
    //
    // 2. make_shared:
    //    T* -> T
    //    ctrlBlock_* -> nullptr
    //    it means that counters lay before T (ControlBlockWithObject)
    struct Counter {
        size_t shared_count_;
        size_t weak_count_;
    };

    // should be U because of T may not
    // to point at the beggining of the object 
    // std::static_pointer_cast
    struct CounterWithObject : Counter{
        T value_;
    };

    T* value_ptr_ = nullptr;
    Counter* ctrl_block_ptr_ = nullptr;
 
    template <typename Y, typename... Args>
    friend SharedPtr<Y> makeShared(Args&&...);
    
    template <typename Y>
    friend class WeakPtr;

public:
    SharedPtr() {}
    SharedPtr(T* ptr)
            : value_ptr_(ptr), ctrl_block_ptr_(new Counter(1, 0)) {
        
        if constexpr (std::is_base_of_v<T, EnableSharedFromThis<T>>) {
            value_ptr_->weak_ptr_ = *this;
        }
    }

    template <typename Del>
    SharedPtr(T* ptr, Del del)
            : value_ptr_(ptr), ctrl_block_ptr_(new Counter(1, 0)) {
        
        if constexpr (std::is_base_of_v<T, EnableSharedFromThis<T>>) {
            value_ptr_->weak_ptr_ = *this;
        }
    }

    template <typename Del, typename Alloc>
    SharedPtr(T* ptr, Del del, Alloc alloc)
            : value_ptr_(ptr), ctrl_block_ptr_(new Counter(1, 0)) {
        
        if constexpr (std::is_base_of_v<T, EnableSharedFromThis<T>>) {
            value_ptr_->weak_ptr_ = *this;
        }
    }

    // Private
    SharedPtr(Counter* ctrl_block_ptr) {
        // TODO
    }

    SharedPtr(const SharedPtr& other)
            : value_ptr_(other.value_ptr_), ctrl_block_ptr_(other.ctrl_block_ptr_) {
        ++ctrl_block_ptr_->shared_count_;
    }
    template <typename U>
    SharedPtr(const SharedPtr<U>& other)
    {
        //TODO
    }

    SharedPtr(SharedPtr&& other)
            : value_ptr_(std::move(other.value_ptr_)), ctrl_block_ptr_(std::move(other.ctrl_block_ptr_)) { }
    template <typename U>
    SharedPtr(SharedPtr<U>&& other)
            : value_ptr_(std::move(other.value_ptr_)), ctrl_block_ptr_(std::move(other.ctrl_block_ptr_)) { }

    SharedPtr& operator=(const SharedPtr& other) {
        if (this != &other) { 
            
            // TODO
        }
        return *this;
    }
    template <typename U>
    SharedPtr& operator=(const SharedPtr<U>& other) {
        // TODO
        return *this;
    }

    SharedPtr& operator=(const SharedPtr&& other) {
        if (this != &other) {    
            // TODO
        }
        return *this;
    }
    template <typename U>
    SharedPtr& operator=(const SharedPtr<U>&& other) {
        
        return *this;
    }

    // TODO: create a SharedPtr from another type
    // auto p = make_shared<Derived>();
    // SharedPtr<Base> bp = p;


    ~SharedPtr() {
        --ctrl_block_ptr_->shared_count_;
        
        if (!ctrl_block_ptr_->shared_count_) {
            value_ptr_->~T();
            
            if (value_ptr_ != reinterpret_cast<T*>(ctrl_block_ptr_ + sizeof(Counter))) {
                // SharedPtr wasn't created using MakeShared()
                delete value_ptr_;
            }
        }

        if (!ctrl_block_ptr_->weak_count_) {
            delete ctrl_block_ptr_;
        }
    }

    T& operator*() const noexcept {
        return *value_ptr_;
    }
    T* operator->() const noexcept {
        return value_ptr_;
    }

    size_t use_count() const noexcept {
        return ctrl_block_ptr_->shared_count_;
    }

    // TODO: may be convertible to T
    void swap(SharedPtr& other) noexcept {
        std::swap(this->value_ptr_, other.value_ptr_);
        std::swap(this->ctrl_block_ptr_, other.ctrl_block_ptr_);
    }
    // rethink
    void swap(SharedPtr&& other) noexcept {
        std::swap(this->value_ptr_, other.value_ptr_);
        std::swap(this->ctrl_block_ptr_, other.ctrl_block_ptr_);
    }

    void reset(T* ptr = nullptr) {
        swap(SharedPtr(ptr));
    }

    T* get() const noexcept {
        return value_ptr_;
    }

private:
    // TODO:
    SharedPtr(CounterWithObject* cp)
            : value_ptr_(&(cp->value_))
            , ctrl_block_ptr_(cp) { }
};

template <typename T, typename... Args>
SharedPtr<T> makeShared(Args&&... args) {
    auto* p = new SharedPtr<T>::CounterWithObject{1, 0, T(std::forward<Args>(args)...)};
    return SharedPtr<T>(p);
}

template<typename T, typename Alloc, typename... Args>
SharedPtr<T> allocateShared(const Alloc& alloc, Args&&... args) {
    return SharedPtr<T>();
}


// weak_ptr, enable_shared_from_this. CRTP
template <typename T>
class WeakPtr {
    SharedPtr<T>::Counter* ctrl_block_ptr_ = nullptr;
public:
    WeakPtr() : ctrl_block_ptr_(nullptr) { }
    WeakPtr(const SharedPtr<T>& shared_ptr)
            : ctrl_block_ptr_(shared_ptr.ctrl_block_ptr_) {
        if (ctrl_block_ptr_)
            ++(ctrl_block_ptr_->weak_count_);
    }

    template <typename U>
    WeakPtr(const WeakPtr<U>& weak_ptr) {
        // TODO
    }

    template <typename U>
    WeakPtr(const SharedPtr<U>& shared_ptr) {
        // TODO
    }

    ~WeakPtr() {
        if (       ctrl_block_ptr_ 
            && !(--ctrl_block_ptr_->weak_count_)
            &&    !ctrl_block_ptr_->shared_count_) { // if outlived shared_ptr
            
            //ctrl_block_ptr_->SharedPtr<T>::~Counter();
            delete ctrl_block_ptr_;
        }
    }

    
    bool expired() const {
        return !ctrl_block_ptr_;
    }

    // TODO:
    SharedPtr<T> lock() const {
        return expired()
            ? SharedPtr<T>()
            // TODO: from *this
            : SharedPtr<T>(this->ctrl_block_ptr_);
    }

    size_t use_count() const noexcept {
        return ctrl_block_ptr_
            ? ctrl_block_ptr_->weak_count_
            : 0;
    }

    template <typename Y, typename Deleter>
    friend class SharedPtr;
};

template <typename T>
class EnableSharedFromThis {
    WeakPtr<T> weak_ptr_;
public:
    EnableSharedFromThis() {}
    SharedPtr<T> shared_from_this() const {
        return weak_ptr_.lock();
    }

    template <typename Y, typename Deleter>
    friend class SharedPtr;
};