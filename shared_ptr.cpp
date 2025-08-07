#include <iostream>
#include <memory>

template <typename T>
struct EnableSharedFromThis;

// TODO: add to all a delegate c-tor
template <typename T, typename Deleter = std::default_delete<T>>
class SharedPtr {

    // Two ways to construct:
    // 1. ctor:
    //    T* -> T
    //    ctrlBlock_* -> CtrlBlock
    //
    // 2. make_shared:
    //    T* -> T
    //    ctrlBlock_* -> CtrlBlockWithObject
    //    it means that counters lay before T (CtrlBlockWithObject)
    struct Counter {
        size_t shared_count_ = 0;
        size_t weak_count_   = 0;
    };

    template <typename U>
    struct CtrlBlock : Counter {
        U*     value_ptr_    = nullptr;
    };

    template <typename U>
    struct CtrlBlockWithObject : CtrlBlock<U> {
        U value_;
    };

    T* value_ptr_                 = nullptr;
    CtrlBlock<T>* ctrl_block_ptr_ = nullptr;
 
    template <typename Y, typename... Args>
    friend SharedPtr<Y> makeShared(Args&&...);
    
    template <typename Y>
    friend class WeakPtr;

    template <typename U, typename DelU>
    friend class SharedPtr;

public:
    SharedPtr()
            : value_ptr_(nullptr)
            , ctrl_block_ptr_(nullptr) {}

    SharedPtr(T* ptr)
            : value_ptr_(ptr)
            , ctrl_block_ptr_(new CtrlBlock({1, 0}, ptr)) {}

    template <typename Del>
    SharedPtr(T* ptr, Del del)
            : value_ptr_(ptr)
            , ctrl_block_ptr_(new CtrlBlock({1, 0}, ptr)) {}

    template <typename Del, typename Alloc>
    SharedPtr(T* ptr, Del del, Alloc alloc)
            : value_ptr_(ptr)
            , ctrl_block_ptr_(new CtrlBlock({1, 0}, ptr)) {}

    SharedPtr(const SharedPtr& other) noexcept
            : value_ptr_(other.value_ptr_)
            , ctrl_block_ptr_(other.ctrl_block_ptr_) {
        if (ctrl_block_ptr_)
            ++(ctrl_block_ptr_->shared_count_);
    }
    SharedPtr(SharedPtr&& other) noexcept
            : value_ptr_(other.value_ptr_)
            , ctrl_block_ptr_(other.ctrl_block_ptr_) {
        other.value_ptr_ = nullptr;
        other.ctrl_block_ptr_ = nullptr;
    }

    template <typename U>
    requires std::is_convertible_v<U*, T*>
    SharedPtr(const SharedPtr<U>& other)
            : value_ptr_(static_cast<T*>(other.value_ptr_))
            , ctrl_block_ptr_(reinterpret_cast<CtrlBlock<T>*>(other.ctrl_block_ptr_)) {
        if (ctrl_block_ptr_)
            ++(ctrl_block_ptr_->shared_count_);
    }

    SharedPtr& operator=(const SharedPtr& other) {
        // check T == other.T
        if (this != &other)
            swap(SharedPtr(other));
        return *this;
    }
    
    SharedPtr& operator=(SharedPtr&& other) noexcept {
        if (this != &other)
            swap(SharedPtr(std::move(other)));
        return *this;
    }

    template <typename U>
    requires std::is_convertible_v<U*, T*>
    SharedPtr& operator=(SharedPtr<U>&& other) noexcept {
        swap(SharedPtr());

        value_ptr_ = static_cast<T*>(other.value_ptr_);
        ctrl_block_ptr_ = reinterpret_cast<CtrlBlock<T>*>(other.ctrl_block_ptr_);

        other.value_ptr_ = nullptr;
        other.ctrl_block_ptr_ = nullptr;

        return *this;
    }

    ~SharedPtr() {
        if (    !ctrl_block_ptr_ 
            || --ctrl_block_ptr_->shared_count_) {
            
            return;
        }
        
        if (value_ptr_)
            value_ptr_->~T();
        
        if (reinterpret_cast<char*>(value_ptr_) - reinterpret_cast<char*>(ctrl_block_ptr_) != sizeof(CtrlBlock<T>)) {
            // was not created using makeShared
            operator delete(value_ptr_);
        }

        if (ctrl_block_ptr_->weak_count_ == 0) {      
            
            if (ctrl_block_ptr_)
                ctrl_block_ptr_->~CtrlBlock();

            operator delete(ctrl_block_ptr_);
        } 
    }

    T& operator*() const noexcept {
        return *get();
    }
    T* operator->() const noexcept {
        return get();
    }

    size_t use_count() const noexcept {
        return ctrl_block_ptr_->shared_count_;
    }

    // TODO: may be convertible to T
    void swap(SharedPtr& other) {
        std::swap(value_ptr_, other.value_ptr_);
        std::swap(ctrl_block_ptr_, other.ctrl_block_ptr_);
    }
    // rethink
    void swap(SharedPtr&& other) {
        std::swap(value_ptr_, other.value_ptr_);
        std::swap(ctrl_block_ptr_, other.ctrl_block_ptr_);
    }

    void reset() noexcept {
        swap(SharedPtr());
    }
    void reset(T* ptr) noexcept {
        swap(SharedPtr(ptr));
    }

    T* get() const noexcept {
        return value_ptr_;
    }

private:
    SharedPtr(CtrlBlockWithObject<T>* ctrl_block_with_object_ptr) 
            : value_ptr_(ctrl_block_with_object_ptr->value_ptr_)
            , ctrl_block_ptr_(ctrl_block_with_object_ptr) {
        ++ctrl_block_ptr_->shared_count_;
    }

    SharedPtr(CtrlBlock<T>* ctrl_block_ptr) 
            : value_ptr_(ctrl_block_ptr->value_ptr_)
            , ctrl_block_ptr_(ctrl_block_ptr) {
        if (ctrl_block_ptr)
            ++ctrl_block_ptr_->shared_count_;
    }
};

template <typename T, typename... Args>
SharedPtr<T> makeShared(Args&&... args) {
    auto* p = new SharedPtr<T>::template CtrlBlockWithObject<T>{0, 0, nullptr, T(std::forward<Args>(args)...)};
    p->value_ptr_ = &p->value_;
    return SharedPtr<T>(p);
}

template<typename T, typename Alloc, typename... Args>
SharedPtr<T> allocateShared(const Alloc& alloc, Args&&... args) {
    return SharedPtr<T>();
}


// weak_ptr, enable_shared_from_this. CRTP
template <typename T>
class WeakPtr {
    using InnerCtrlBlock = SharedPtr<T>::template CtrlBlock<T>;

    InnerCtrlBlock* ctrl_block_ptr_;

    template <typename U>
    friend class WeakPtr;
    template <typename U, typename Deleter>
    friend class SharedPtr;
public:
    WeakPtr(const SharedPtr<T>& shared_ptr = SharedPtr<T>())
            : ctrl_block_ptr_(shared_ptr.ctrl_block_ptr_) {
        if (ctrl_block_ptr_)
            ++(ctrl_block_ptr_->weak_count_);
    }
    WeakPtr(const WeakPtr<T>& other)
            : ctrl_block_ptr_(other.ctrl_block_ptr_) {
        if (ctrl_block_ptr_)
            ++(ctrl_block_ptr_->weak_count_);
    }

    template <typename U>
    requires std::is_convertible_v<U*, T*>
    WeakPtr(const WeakPtr<U>& other)
            : ctrl_block_ptr_(reinterpret_cast<InnerCtrlBlock*>(other.ctrl_block_ptr_)) {
        if (ctrl_block_ptr_)
            ++(ctrl_block_ptr_->weak_count_);
    }
    template <typename U>
    requires std::is_convertible_v<U*, T*>
    WeakPtr(const SharedPtr<U>& other)
            : ctrl_block_ptr_(reinterpret_cast<InnerCtrlBlock*>(other.ctrl_block_ptr_)) {
        if (ctrl_block_ptr_)
            ++(ctrl_block_ptr_->weak_count_);
    }
    
    WeakPtr(WeakPtr<T>&& other)
            : ctrl_block_ptr_(other.ctrl_block_ptr_) {
        other.ctrl_block_ptr_ = nullptr;
    }

    WeakPtr& operator=(const SharedPtr<T>& shared_ptr) {
        WeakPtr<T> temp = shared_ptr;
        swap(std::move(temp));
        return *this;
    }

    ~WeakPtr() {
        if (    !ctrl_block_ptr_
            || --ctrl_block_ptr_->weak_count_)
            return;

        if (ctrl_block_ptr_->shared_count_)
            return;

        operator delete(ctrl_block_ptr_);
    }

    
    bool expired() const noexcept {
        return !use_count();
    }

    SharedPtr<T> lock() const noexcept{
        return expired()
            ? SharedPtr<T>()
            : SharedPtr<T>(ctrl_block_ptr_);
    }

    size_t use_count() const noexcept {
        return ctrl_block_ptr_->shared_count_;
    }

    void swap(WeakPtr&& other) {
        std::swap(ctrl_block_ptr_, other.ctrl_block_ptr_);
    }
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