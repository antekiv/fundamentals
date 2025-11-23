// List, SimpleAllocator, StackAllocator
#include <utility>
#include <array>
#include <memory>
#include <iostream>
#include <iterator>

template <size_t N>
using StackStorage = std::array<char, N>;

template <typename T>
struct SimpleAllocator {
    using value_type = T;
    SimpleAllocator() = default;

    T* allocate(size_t count) {
        // execute the first part of operator new (return count * sizeof(T) bytes)
        return static_cast<T*>(::operator new(count * sizeof(T)));
    }
    void deallocate(T* ptr, size_t) {
        // execute the second part of operator delete (deallocate n bytes [count_of_bytes][[*ptr] buffer]) 
        operator delete(ptr);
    }

    template <typename U, typename... Args>
    void construct(U* ptr, Args&&... args) {
        new (ptr) U(std::forward<Args>(args)...);
    }

    template <typename U>
    void destroy(U* ptr) {
        ptr->~U();
    }

    template <typename U>
    SimpleAllocator(SimpleAllocator<U>){}

    template <typename U>
    struct rebind {
        using other = SimpleAllocator<U>;
    };
};


template <typename T, size_t N>
struct StackAllocator {
    using value_type = T;

    StackAllocator(StackStorage<N>& pool) 
        : ptr_(std::make_shared<void*>(pool.begin())) {}

    T* allocate(size_t count) {
        size_t bytes_needed = count * sizeof(T);
        size_t alignment = alignof(T);

        T* alignmend_ptr = reinterpret_cast<T*>(std::align(alignment, bytes_needed, *ptr_, *space_remaining_));
        *ptr_ = alignmend_ptr + bytes_needed;
        return alignmend_ptr;
    }
    void deallocate(T* ptr, size_t) {
        //operator delete(ptr);
    }

    template <typename U, typename... Args>
    void construct(U* ptr, Args&&... args) {
        new (ptr) U(std::forward<Args>(args)...);
    }

    // for example list
    template <typename U>
    void destroy(U* ptr) {
        ptr->~U();
    }

    template <typename U>
    StackAllocator(const StackAllocator<U, N>& other) 
        : ptr_(other.ptr_)
        , space_remaining_(other.space_remaining_)
    {}

    template <typename U>
    struct rebind {
        using other = StackAllocator<U, N>;
    };

private:
public:
    // std::byte?
    std::shared_ptr<void*> ptr_;
    std::shared_ptr<size_t> space_remaining_ = std::make_shared<size_t>(N);
};

template <typename T,
          typename Allocator = std::allocator<T>>
class List {    
    struct BaseNode {
        BaseNode* prev;
        BaseNode* next;
    };
    struct Node : BaseNode {
        T value;
    };

    BaseNode fake_node_;
    size_t sz_;
    [[no_unique_address]]
    typename std::allocator_traits<Allocator>::template rebind_alloc<Node> alloc_;

private:
    template <bool IsConst>
    class base_iterator {
    public:
        using difference_type = std::ptrdiff_t;
        using iterator_category = std::bidirectional_iterator_tag;
        using pointer_type = std::conditional_t<IsConst, const T*, T*>;
        using reference_type = std::conditional_t<IsConst, const T&, T&>;
        using value_type = std::conditional_t<IsConst, const T, T>;

    private:
        friend class List<T, Allocator>;
        
        using base_node_pointer = std::conditional_t<IsConst, const BaseNode*, BaseNode*>;
        using node_pointer = std::conditional_t<IsConst, const Node*, Node*>;

        base_node_pointer ptr_;

        base_iterator(base_node_pointer ptr): ptr_(ptr) {}
    public:        
        base_iterator(const base_iterator&) = default;

        base_iterator& operator=(const base_iterator& other) = default;

        bool operator==(const base_iterator& other) const {
            return ptr_ == other.ptr_;
        }
        
        reference_type operator*() const {return static_cast<node_pointer>(ptr_)->value;};
        pointer_type operator->() const {return &(static_cast<node_pointer>(ptr_)->value);}

        base_iterator& operator++() {
            ptr_ = ptr_->next;
            return *this;
        }
        const base_iterator operator++(int) {
            base_iterator copy = *this;
            ptr_ = ptr_->next;
            return copy;
        }

        base_iterator& operator--() {
            ptr_ = ptr_->prev;
            return *this;
        }
        const base_iterator operator--(int) {
            base_iterator copy = *this;
            ptr_ = ptr_->prev;
            return copy;
        }

        base_iterator operator+(size_t n) {
            base_iterator iter = *this;
            while (n--) { ++iter;}
            return iter;
        }

        base_iterator operator-(size_t n) {
            base_iterator iter = *this;
            while (n--) { --iter;}
            return iter;
        }

        operator base_iterator<true>() const {
            return {ptr_};
        }
    };
public:
    using const_iterator = base_iterator<true>;
    using iterator = base_iterator<false>;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    iterator begin() {
        return {fake_node_.next};
    }

    iterator end() {
        // decrement to end() should get the last element
        return {&fake_node_};
    }

    const_iterator begin() const {
        return const_iterator{fake_node_.next};
    }

    const_iterator end() const {
        return const_iterator{&fake_node_};
    }

    const_iterator cbegin() const {
        return const_iterator{fake_node_.next};
    }

    const_iterator cend() const {
        return const_iterator{&fake_node_};
    }

    // reverse_iterators
    reverse_iterator rbegin() {
        return reverse_iterator{&fake_node_};
    }

    reverse_iterator rend() {
        return reverse_iterator{fake_node_.next};
    }

    const_reverse_iterator rbegin() const {
        return const_reverse_iterator{&fake_node_};
    }

    const_reverse_iterator rend() const {
        return const_reverse_iterator{fake_node_.next};
    }

    const_reverse_iterator crbegin() const {
        return const_reverse_iterator{&fake_node_};
    }

    const_reverse_iterator crend() const {
        return const_reverse_iterator{fake_node_.next};
    }

public:
    using AllocTraits = std::allocator_traits<decltype(alloc_)>;
    explicit List(const Allocator& alloc = Allocator()) 
        : fake_node_{&fake_node_, &fake_node_}
        , sz_{0}
        , alloc_(alloc) {}

     
    explicit List(size_t count, const Allocator& alloc = Allocator()) 
        : fake_node_{&fake_node_, &fake_node_}
        , sz_{0}
        , alloc_(alloc) {

        // another try? 
        try {
            for (size_t i = 0; i < count; ++i) {
                insert(this->cend());
            }       
        } catch (...) {
            remove_all_elements();
            throw;
        }
    }
    
    List(const List& other) 
        : fake_node_{&fake_node_, &fake_node_}
        , sz_{0}
        // propagate_on_copy_assignable
        , alloc_(other.alloc_) {
        
        try {
            for (const auto& el : other) {
                this->push_back(el);
            }
        } catch (...) {
            remove_all_elements();
            throw;
        }
    }

    List& operator=(const List& other) {
        if (this != &other) {
            size_t size = sz_;
            try {
                for (const auto& e : other) {
                    push_back(e);
                }
            } catch (...) {
                while (this->size() != size)
                    pop_back();

                throw;
            }

            while (size--) {
                pop_front();
            }
        }
        return *this;
    }

    ~List() {
        remove_all_elements();
    }

    template <typename... Args>
    [[maybe_unused]] iterator insert(const_iterator pos, Args&&... args) {
        Node* new_node = create_node(std::forward<Args>(args)...);
        
        BaseNode* current = const_cast<BaseNode*>(pos.ptr_);

        rebind_nodes(current, new_node);
        ++sz_;
        return iterator {new_node};
    }

    void push_back(const T& val) {
        insert(cend(), val);
    }

    void push_front(const T& val) {
        insert(cbegin(), val);
    }

    [[maybe_unused]] iterator erase(const_iterator pos) {
        Node* curr = static_cast<Node*>(const_cast<BaseNode*>(pos.ptr_));
        Node* prev = static_cast<Node*>(curr->prev);

        auto next = delete_node(curr);
        bind_nodes(prev, next);

        return iterator(next);
    }

    void pop_front() {
        erase(cbegin());
    }

    void pop_back() {
        erase(cend() - 1);
    }

    size_t size() const {
        return sz_;
    } 

private:
    template <typename... Args>
    Node* create_node(Args&&... args) {
        Node* new_arr = AllocTraits::allocate(alloc_, 1);
        try {
            AllocTraits::construct(alloc_, &(new_arr->value), std::forward<Args>(args)...);
        } catch (...) {
            AllocTraits::deallocate(alloc_, new_arr, 1);
            throw;
        }
        return new_arr;
    }

    Node* delete_node(Node* node) {
        Node* ret = static_cast<Node*>(node->next);
        
        AllocTraits::destroy(alloc_, &(node->value));
        AllocTraits::deallocate(alloc_, node, 1);
        --sz_;

        return ret;
    }

    void bind_nodes(BaseNode* lhs, BaseNode* rhs) {
        lhs->next = rhs;
        rhs->prev = lhs;
    }

    void rebind_nodes(BaseNode* pos, BaseNode* new_node) {        
        BaseNode* prev_node = pos->prev;

        prev_node->next = new_node;
        new_node->prev = prev_node;
        new_node->next = pos;
        pos->prev = new_node;
    }

    void remove_all_elements(){
        auto it = begin();
        while (it != end()) {
            it = erase(cbegin());
        }
    } 
};