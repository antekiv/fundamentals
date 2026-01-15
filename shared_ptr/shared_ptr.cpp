#include <iostream>
#include <memory>

template <typename T>
struct EnableSharedFromThis;

namespace {
    struct VirtualControlBlockBase {
        size_t shared_count_ = 1;
        size_t weak_count_   = 0;
        
        virtual ~VirtualControlBlockBase() = default;
        virtual void dispose() = 0;
        virtual void destroy() = 0;
    };
}
// TODO: add to all a delegate c-tor
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
        U value_;
        [[no_unique_address]] UAlloc alloc_;

        CtrlBlockMakeShared(const U& value, UAlloc alloc = std::allocator<U>())
            : value_(value)
            , alloc_(std::move(alloc)) {}

        CtrlBlockMakeShared(U&& value, UAlloc alloc = std::allocator<U>())
            : value_(std::move(value))
            , alloc_(std::move(alloc)) {}

        void dispose() override {
            std::allocator_traits<UAlloc>::destroy(alloc_, &value_);
        }

        void destroy() override {
            using AllocTraits = std::allocator_traits<UAlloc>;
            using BlockAlloc = typename AllocTraits::template rebind_alloc<CtrlBlockMakeShared>;
            BlockAlloc ba = alloc_;
            this->~CtrlBlockHolder();
            AllocTraits::deallocate(ba, this, 1);
        }
        
        ~CtrlBlockMakeShared() {
            // Пустой деструктор, так как T разрушается в dispose()
        }
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
            , ctrl_block_ptr_(new CtrlBlock(ptr, std::default_delete<T>(), std::allocator<T>())) {}
    
    template <typename Del>
    SharedPtr(T* ptr, Del del)
            : value_ptr_(ptr)
            , ctrl_block_ptr_(new CtrlBlock(ptr, std::default_delete<T>(), std::allocator<T>())) {}

    template <typename Del, typename Alloc>
    SharedPtr(T* ptr, Del del, Alloc alloc)
            : value_ptr_(ptr)
            , ctrl_block_ptr_(new CtrlBlock(ptr, std::default_delete<T>(), std::allocator<T>())) {}
    

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
            : value_ptr_(other.value_ptr_)
            , ctrl_block_ptr_(other.ctrl_block_ptr_) {
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

    /*
    template <typename U>
    requires std::is_convertible_v<U*, T*>
    SharedPtr& operator=(SharedPtr<U>&& other) noexcept {
        swap(SharedPtr());

        value_ptr_ = static_cast<T*>(other.value_ptr_);
        ctrl_block_ptr_ = reinterpret_cast<VirtualControlBlockBase<T>*>(other.ctrl_block_ptr_);

        other.value_ptr_ = nullptr;
        other.ctrl_block_ptr_ = nullptr;

        return *this;
    }
    */

    ~SharedPtr() {
        if (!ctrl_block_ptr_)
            return;
        
        if (--ctrl_block_ptr_->shared_count_ == 0) {
            ctrl_block_ptr_->dispose();
            if (ctrl_block_ptr_->weak_count_ == 0) {
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

    SharedPtr(CtrlBlockMakeShared<T, std::allocator<T>>* ctrl_block_with_object_ptr) 
            : value_ptr_(&(ctrl_block_with_object_ptr->value_))
            , ctrl_block_ptr_(ctrl_block_with_object_ptr) {
        ++ctrl_block_ptr_->shared_count_;
    }

    SharedPtr(VirtualControlBlockBase* ctrl_block_ptr) 
            : value_ptr_(ctrl_block_ptr->value_ptr_)
            , ctrl_block_ptr_(ctrl_block_ptr) {
        if (ctrl_block_ptr)
            ++ctrl_block_ptr_->shared_count_;
    }
/*
    template <typename Alloc, typename... Args>
    SharedPtr(const Alloc& alloc, Args&&... args) {
        using AllocatorTraits = std::allocator_traits<Alloc>;

        // may be rebind
        auto new_alloc = alloc;
        T* new_arr = AllocatorTraits::allocate(new_alloc, 1);
        
        try { 
            AllocatorTraits::construct(new_alloc, new_arr, std::forward<Args>(args)...);
        } catch (...) {
            AllocatorTraits::deallocate(new_alloc, new_arr, 1);
            throw;
        }

        // TODO using one new
        value_ptr_ = new_arr;
        ctrl_block_ptr_ = new CtrlBlock{{1, 0}, std::default_delete<T>(), new_arr};
    }
        */
};

template <typename T, typename... Args>
SharedPtr<T> makeShared(Args&&... args) {
    auto* p = new SharedPtr<T>::template CtrlBlockMakeShared<T, std::allocator<T>>{T(std::forward<Args>(args)...), std::allocator<T>()};
    return SharedPtr<T>(p);
}

template<typename T, typename Alloc>
using ReboundAlloc = typename std::allocator_traits<Alloc>::template rebind_alloc<T>;

// https://github.com/microsoft/STL/blob/5f8b52546480a01d1d9be6c033e31dfce48d4f13/stl/inc/memory#L3025C44-L3025C59
/*
template<typename T, typename Alloc, typename... Args>
SharedPtr<T> allocateShared(const Alloc& alloc, Args&&... args) {
    return SharedPtr<T>(alloc, std::forward<Args>(args)...);
}
*/


// weak_ptr, enable_shared_from_this. CRTP
template <typename T>
class WeakPtr {
    using InnerCtrlBlock = VirtualControlBlockBase;

    InnerCtrlBlock* ctrl_block_ptr_;

    template <typename U>
    friend class WeakPtr;

    template <typename U>
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
        if (!ctrl_block_ptr_)
            return;

        if (--ctrl_block_ptr_->weak_count_ == 0) {
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

    template <typename Y>
    friend class SharedPtr;
};


// template <class _Ty>
// class _Ref_count : public _Ref_count_base { // handle reference counting for pointer without deleter

// template <class _Resource, class _Dx>
// class _Ref_count_resource : public _Ref_count_base { // handle reference counting for object with deleter
