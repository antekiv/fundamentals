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
    struct CtrlBlock {
        size_t shared_count_ = 0;
        size_t weak_count_   = 0;
        T*     value_ptr_    = nullptr;
    };

    // should be U because of T may not
    // to point at the beggining of the object 
    // std::static_pointer_cast
    struct CtrlBlockWithObject : CtrlBlock{
        T value_;
    };

    T* value_ptr_ = nullptr;
    CtrlBlock* ctrl_block_ptr_ = nullptr;
 
    template <typename Y, typename... Args>
    friend SharedPtr<Y> makeShared(Args&&...);
    
    template <typename Y>
    friend class WeakPtr;

public:
    SharedPtr(T* ptr = nullptr)
            : value_ptr_(ptr)
            , ctrl_block_ptr_(new CtrlBlock(1, 0, ptr)) {}

    template <typename Del>
    SharedPtr(T* ptr, Del del)
            : value_ptr_(ptr), ctrl_block_ptr_(new CtrlBlock(1, 0, ptr)) {}

    template <typename Del, typename Alloc>
    SharedPtr(T* ptr, Del del, Alloc alloc)
            : value_ptr_(ptr)
            , ctrl_block_ptr_(new CtrlBlock(1, 0, ptr)) {}

    SharedPtr(const SharedPtr& other) noexcept
            : value_ptr_(other.value_ptr_)
            , ctrl_block_ptr_(other.ctrl_block_ptr_) {
        ++(ctrl_block_ptr_->shared_count_);
    }
    SharedPtr(SharedPtr&& other) noexcept
            : value_ptr_(other.value_ptr_)
            , ctrl_block_ptr_(other.ctrl_block_ptr_) {
        other.value_ptr_ = nullptr;
        other.ctrl_block_ptr_ = new CtrlBlock(1, 0, nullptr);
    }

    template <typename U>
    SharedPtr(const SharedPtr<U>& other) {
        //TODO
    }

    // REDO
    template <typename U>
    SharedPtr(SharedPtr<U>&& other)
            : value_ptr_(std::move(other.value_ptr_))
            , ctrl_block_ptr_(std::move(other.ctrl_block_ptr_)) { }

    SharedPtr& operator=(const SharedPtr& other) {
        if (this != &other)
            swap(SharedPtr(other));
        return *this;
    }
    template <typename U>
    SharedPtr& operator=(const SharedPtr<U>& other) {
        // TODO
        return *this;
    }

    SharedPtr& operator=(SharedPtr&& other) noexcept {
        if (this != &other)
            swap(SharedPtr(std::move(other)));
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
        if (--ctrl_block_ptr_->shared_count_)
            return;
        
        if (!ctrl_block_ptr_->weak_count_) {
            if constexpr (!std::is_base_of_v<T, EnableSharedFromThis<T>>) {
                delete value_ptr_;
            }
                
            delete ctrl_block_ptr_;
        } else {
            delete value_ptr_;
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
        std::swap(this->value_ptr_, other.value_ptr_);
        std::swap(this->ctrl_block_ptr_, other.ctrl_block_ptr_);
    }
    // rethink
    void swap(SharedPtr&& other){
        std::swap(this->value_ptr_, other.value_ptr_);
        std::swap(this->ctrl_block_ptr_, other.ctrl_block_ptr_);
    }

    void reset(T* ptr = nullptr) noexcept {
        swap(SharedPtr(ptr));
    }

    T* get() const noexcept {
        return value_ptr_;
    }

private:
    // TODO:
    SharedPtr(CtrlBlock* ctrl_block_ptr) 
            : value_ptr_(ctrl_block_ptr->value_ptr_)
            , ctrl_block_ptr_(ctrl_block_ptr) {
        ++ctrl_block_ptr_->shared_count_;
    }
};

template <typename T, typename... Args>
SharedPtr<T> makeShared(Args&&... args) {
    auto* p = new SharedPtr<T>::CtrlBlockWithObject{1, 0, nullptr, T(std::forward<Args>(args)...)};
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
    SharedPtr<T>::CtrlBlock* ctrl_block_ptr_;
public:
    WeakPtr(const SharedPtr<T>& shared_ptr = SharedPtr<T>())
            : ctrl_block_ptr_(shared_ptr.ctrl_block_ptr_) {
        ++(ctrl_block_ptr_->weak_count_);
    }
    WeakPtr(const WeakPtr<T>& other)
            : ctrl_block_ptr_(other.ctrl_block_ptr_) {
        ++(ctrl_block_ptr_->weak_count_);
    }
    WeakPtr(WeakPtr<T>&& other)
            : ctrl_block_ptr_(other.ctrl_block_ptr_) {
        // Hmm... maybe need to add checking nullptr for ctrl_block
        other.ctrl_block_ptr_ = new SharedPtr<T>::CtrlBlock(0, 1, nullptr); 
    }

    WeakPtr& operator=(const SharedPtr<T>& shared_ptr) {
        WeakPtr<T> temp = shared_ptr;
        swap(std::move(temp));
        return *this;
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
        if (--ctrl_block_ptr_->weak_count_)
            return;

        if (ctrl_block_ptr_->shared_count_)
            return;

        delete ctrl_block_ptr_;
    }

    
    bool expired() const noexcept {
        return !use_count();
    }

    SharedPtr<T> lock() const noexcept{
        return expired()
            ? SharedPtr<T>()
            : SharedPtr<T>(this->ctrl_block_ptr_);
    }

    size_t use_count() const noexcept {
        return ctrl_block_ptr_->shared_count_;
    }

    void swap(WeakPtr&& other) {
        std::swap(this->ctrl_block_ptr_, other.ctrl_block_ptr_);
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