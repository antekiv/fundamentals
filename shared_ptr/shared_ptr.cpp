#include <iostream>
#include <memory>

template <typename T>
struct EnableSharedFromThis;

template <typename T>
struct WeakPtr;

namespace {
    struct VirtualControlBlockBase {
        size_t shared_count_ = 1;
        size_t weak_count_   = 0;
        
        virtual ~VirtualControlBlockBase() = default;
        virtual void dispose() = 0;
        virtual void destroy() = 0;
    };
}

template <typename T>
class SharedPtr {
    template <typename U, typename UDel, typename UAlloc>
    struct CtrlBlock : public VirtualControlBlockBase {
        U* value_ptr_;
        [[no_unique_address]] UDel deleter_;
        [[no_unique_address]] UAlloc alloc_;

        CtrlBlock(U* ptr, UDel del = std::default_delete<T>(), UAlloc alloc = std::allocator<T>())
            : value_ptr_(ptr)
            , deleter_(std::move(del))
            , alloc_(std::move(alloc)) {}

        void dispose() override {
            deleter_(value_ptr_);
        }

        void destroy() override {
            using AllocTraits = std::allocator_traits<UAlloc>;
            using BlockAlloc = typename AllocTraits::template rebind_alloc<CtrlBlock>;
            using BlockTraits = std::allocator_traits<BlockAlloc>;

            BlockAlloc ba = alloc_;
            this->~CtrlBlock();
            BlockTraits::deallocate(ba, this, 1);
        }
    };

    template <typename U, typename UAlloc>
    struct CtrlBlockMakeShared : public VirtualControlBlockBase {
        union { U value_; };
        [[no_unique_address]] UAlloc alloc_;

        template <typename... Args>
        CtrlBlockMakeShared(const UAlloc& alloc, Args&&... args) 
            : value_(std::forward<Args>(args)...)
            , alloc_(alloc)
        {
            this->shared_count_ = 1; 
            this->weak_count_ = 0;
        }

        void dispose() override {
            std::allocator_traits<UAlloc>::destroy(alloc_, &value_);
        }

        void destroy() override {
            using AllocTraits = std::allocator_traits<UAlloc>;
            using BlockAlloc = typename AllocTraits::template rebind_alloc<CtrlBlockMakeShared>;
            using BlockTraits = std::allocator_traits<BlockAlloc>;

            BlockAlloc ba = alloc_;
            this->~CtrlBlockMakeShared();
            BlockTraits::deallocate(ba, this, 1);
        }
        
        ~CtrlBlockMakeShared() {}
    };

    T*                       value_ptr_      = nullptr;
    VirtualControlBlockBase* ctrl_block_ptr_ = nullptr;
 

    template <typename U, typename... Args>
    friend SharedPtr<U> makeShared(Args&&...);

    template<typename U, typename Alloc, typename... Args>
    friend SharedPtr<U> allocateShared(const Alloc& alloc, Args&&... args);

    template <typename U>
    friend class SharedPtr;

    template <typename Y>
    friend class WeakPtr;
public:
    SharedPtr()
            : value_ptr_(nullptr)
            , ctrl_block_ptr_(nullptr) {}

    SharedPtr(T* ptr)
            : value_ptr_(ptr)
            , ctrl_block_ptr_(new CtrlBlock(ptr, std::default_delete<T>(), std::allocator<T>())) {

        if constexpr (std::is_base_of_v<EnableSharedFromThis<T>, T>) {
            ptr->weak_ptr_ = *this;
        }
    }
    
    template <typename Del>
    SharedPtr(T* ptr, Del del)
            : value_ptr_(ptr)
            , ctrl_block_ptr_(new CtrlBlock(ptr, std::move(del), std::allocator<T>())) {}

    template <typename Del, typename Alloc>
    SharedPtr(T* ptr, Del del, Alloc alloc)
            : value_ptr_(ptr) {
        
        using ControlBlock = CtrlBlock<T, Del, Alloc>;
        using AllocTraits = std::allocator_traits<Alloc>;
        using BlockAlloc = typename AllocTraits::template rebind_alloc<ControlBlock>;
        using BlockTraits = std::allocator_traits<BlockAlloc>;

        BlockAlloc ba(alloc);
        ControlBlock* block = nullptr;

        try {
            block = BlockTraits::allocate(ba, 1);
            new (block) ControlBlock(ptr, std::move(del), alloc);
            
            ctrl_block_ptr_ = block;

            if constexpr (std::is_base_of_v<EnableSharedFromThis<T>, T>) {
                ptr->weak_ptr_ = *this;
            }
            
        } catch (...) {
            if (block) {
                BlockTraits::deallocate(ba, block, 1); 
            }
            del(ptr);
            throw;
        }
    }

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
    SharedPtr(SharedPtr<U>&& other) noexcept
            : value_ptr_(static_cast<T*>(other.value_ptr_))
            , ctrl_block_ptr_(other.ctrl_block_ptr_) {
        other.value_ptr_ = nullptr;
        other.ctrl_block_ptr_ = nullptr;
    }

    template <typename U>
    requires std::is_convertible_v<U*, T*>
    SharedPtr(const SharedPtr<U>& other)
            : value_ptr_(other.value_ptr_)
            , ctrl_block_ptr_(other.ctrl_block_ptr_) {
        if (ctrl_block_ptr_)
            ++(ctrl_block_ptr_->shared_count_);
    }

    SharedPtr& operator=(const SharedPtr& other) {
        SharedPtr(other).swap(*this);
        return *this;
    }
    
    SharedPtr& operator=(SharedPtr&& other) noexcept {
        SharedPtr(std::move(other)).swap(*this);
        return *this;
    }

    template <typename U>
    requires std::is_convertible_v<U*, T*>
    SharedPtr& operator=(SharedPtr<U>&& other) noexcept {
        SharedPtr(std::move(other)).swap(*this);
        return *this;
    }

    ~SharedPtr() {
        if (!ctrl_block_ptr_)
            return;
    
        if (--ctrl_block_ptr_->shared_count_ == 0) {
            ++ctrl_block_ptr_->weak_count_;
        
            ctrl_block_ptr_->dispose();
        
            if (--ctrl_block_ptr_->weak_count_ == 0) {
                ctrl_block_ptr_->destroy();
            }
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
    void reset() noexcept {
        SharedPtr().swap(*this);
    }
    void reset(T* ptr) noexcept {
        SharedPtr(ptr).swap(*this);
    }

    T* get() const noexcept {
        return value_ptr_;
    }

    operator bool() const noexcept {
        return ctrl_block_ptr_;
    }

private:

    template <typename UAlloc>
    SharedPtr(CtrlBlockMakeShared<T, UAlloc>* ctrl_block_with_object_ptr) 
            : value_ptr_(&(ctrl_block_with_object_ptr->value_))
            , ctrl_block_ptr_(ctrl_block_with_object_ptr) {

        if constexpr (std::is_base_of_v<EnableSharedFromThis<T>, T>) {
            value_ptr_->weak_ptr_ = *this;
        }
    }
};

template<typename T, typename Alloc, typename... Args>
SharedPtr<T> allocateShared(const Alloc& alloc, Args&&... args) {
    using ControlBlock = typename SharedPtr<T>::template CtrlBlockMakeShared<T, Alloc>;
    using AllocTraits = std::allocator_traits<Alloc>;
    using BlockAlloc = typename AllocTraits::template rebind_alloc<ControlBlock>;
    using BlockTraits = std::allocator_traits<BlockAlloc>;

    BlockAlloc blockAlloc(alloc);

    ControlBlock* ptr = BlockTraits::allocate(blockAlloc, 1);

    try {
        BlockTraits::construct(blockAlloc, ptr, blockAlloc, std::forward<Args>(args)...);
    } catch (...) {
        BlockTraits::deallocate(blockAlloc, ptr, 1);
        throw;
    }
    return SharedPtr<T>(ptr); 
}

template <typename T, typename... Args>
SharedPtr<T> makeShared(Args&&... args) {
    return allocateShared<T>(std::allocator<T>(), std::forward<Args>(args)...);
}

// weak_ptr, enable_shared_from_this. CRTP
template <typename T>
class WeakPtr {
    using InnerCtrlBlock = VirtualControlBlockBase;

    InnerCtrlBlock* ctrl_block_ptr_;
    T* value_ptr_;

    template <typename U>
    friend class WeakPtr;

    template <typename U>
    friend class SharedPtr;
public:
    WeakPtr(const SharedPtr<T>& shared_ptr = SharedPtr<T>())
            : ctrl_block_ptr_(shared_ptr.ctrl_block_ptr_)
            , value_ptr_(shared_ptr.value_ptr_) {
        if (ctrl_block_ptr_)
            ++(ctrl_block_ptr_->weak_count_);
    }
    WeakPtr(const WeakPtr<T>& other)
            : ctrl_block_ptr_(other.ctrl_block_ptr_)
            , value_ptr_(other.value_ptr_) {
        if (ctrl_block_ptr_)
            ++(ctrl_block_ptr_->weak_count_);
    }

    template <typename U>
    requires std::is_convertible_v<U*, T*>
    WeakPtr(const WeakPtr<U>& other)
            : ctrl_block_ptr_(other.ctrl_block_ptr_)
            , value_ptr_(other.value_ptr_) {
        if (ctrl_block_ptr_)
            ++(ctrl_block_ptr_->weak_count_);
    }
    template <typename U>
    requires std::is_convertible_v<U*, T*>
    WeakPtr(const SharedPtr<U>& other)
            : ctrl_block_ptr_(other.ctrl_block_ptr_)
            , value_ptr_(other.value_ptr_) {
        if (ctrl_block_ptr_)
            ++(ctrl_block_ptr_->weak_count_);
    }
    
    WeakPtr(WeakPtr<T>&& other)
            : ctrl_block_ptr_(other.ctrl_block_ptr_)
            , value_ptr_(other.value_ptr_) {
        other.ctrl_block_ptr_ = nullptr;
        other.value_ptr_ = nullptr;
    }

    WeakPtr& operator=(const SharedPtr<T>& shared_ptr) {
        WeakPtr<T> temp = shared_ptr;
        swap(temp);
        return *this;
    }

    ~WeakPtr() {
        if (!ctrl_block_ptr_)
            return;

        if (--ctrl_block_ptr_->weak_count_ == 0 
           && ctrl_block_ptr_->shared_count_ == 0) {
            ctrl_block_ptr_->destroy();
        }
    }

    
    bool expired() const noexcept {
        return !use_count();
    }

    SharedPtr<T> lock() const noexcept{
        if (ctrl_block_ptr_ && ctrl_block_ptr_->shared_count_ > 0) {
            SharedPtr<T> sp;
            sp.value_ptr_ = value_ptr_;
            sp.ctrl_block_ptr_ = ctrl_block_ptr_;
            ++ctrl_block_ptr_->shared_count_;
            return sp;
        }
        return SharedPtr<T>();
    }

    size_t use_count() const noexcept {
        return ctrl_block_ptr_
            ? ctrl_block_ptr_->shared_count_
            : 0;
    }

    void swap(WeakPtr& other) noexcept {
        std::swap(ctrl_block_ptr_, other.ctrl_block_ptr_);
        std::swap(value_ptr_, other.value_ptr_);
    }
};


template <typename T>
class EnableSharedFromThis {
    WeakPtr<T> weak_ptr_;
public:
    EnableSharedFromThis() {}
    SharedPtr<T> shared_from_this() const {
        SharedPtr<T> ptr = weak_ptr_.lock();
        if (!ptr) {
            throw std::bad_weak_ptr(); 
        }
        return ptr;
    }

    template <typename Y>
    friend class SharedPtr;
};