// List realization

#include <memory>
#include <iostream>

template <typename T,
          typename Allocator = std::allocator<T>>
class List {
    using size_type = unsigned long long;
    
    struct BaseNode {
        BaseNode* prev;
        BaseNode* next;
    };
    struct Node : BaseNode {
        T value;
    };

    BaseNode fake_node_;
    size_type sz_;
    [[no_unique_address]]
    typename std::allocator_traits<Allocator>::template rebind_alloc<Node> alloc_;


private:
    template <bool IsConst>
    class base_iterator {
        friend void List<T, Allocator>::rebind_nodes(base_iterator<false>, BaseNode*);
        friend void List<T, Allocator>::bind_nodes(base_iterator<false>, base_iterator<false>);
    public:
        using pointer_type = std::conditional_t<IsConst, const T*, T*>;
        using reference_type = std::conditional_t<IsConst, const T&, T&>;
        using value_type = T;
    private:
        BaseNode* ptr_;
        
    public:
        base_iterator(BaseNode* ptr): ptr_(ptr) {}
        base_iterator(const base_iterator&) = default;
        base_iterator& operator=(const base_iterator& other) = default;

        bool operator==(const base_iterator& other) const {
            return ptr_ == other.ptr_;
        }
        
        reference_type operator*() const {return static_cast<Node*>(ptr_)->value;};
        pointer_type operator->() const {return &(static_cast<Node*>(ptr_)->value);}

        base_iterator& operator++() {
            ptr_ = ptr_->next;
            return *this;
        }
        base_iterator operator++(int) {
            base_iterator copy = *this;
            ptr_ = ptr_->next;
            return copy;
        }

        base_iterator& operator--() {
            ptr_ = ptr_->prev;
            return *this;
        }
        base_iterator operator--(int) {
            base_iterator copy = *this;
            ptr_ = ptr_->prev;
            return copy;
        }

        base_iterator<true> to_const() const {
            return base_iterator<true>(ptr_);
        }
    };
public:
    using const_iterator = base_iterator<true>;
    using iterator = base_iterator<false>;
    
    iterator begin() {
        return {fake_node_.next};
    }

    iterator end() {
        // decrement to end() should get the last element
        return {&fake_node_};
    }

    
    const_iterator begin() const {
        return iterator{fake_node_.next}.to_const();
    }

    const_iterator end() const {
        return iterator{&fake_node_}.to_const();
    }

    const_iterator cbegin() const {
        return const_iterator{fake_node_.next};
    }

    const_iterator cend() const {
        std::cout << "cend.fake_node_.prev: " << &fake_node_ << std::endl;
        return const_iterator{&fake_node_};
    }

    /* TODO:
    const_iterator cbegin() const {
        return {arr_};
    }

    const_iterator cend() const {
        return {arr_ + sz_};
    }
    */

public:
    using AllocTraits = std::allocator_traits<decltype(alloc_)>;
    List() 
        : fake_node_{&fake_node_, &fake_node_}
        , sz_{0} {}

    ~List() {
        BaseNode* node = fake_node_.next;
        for (; node != end();) {
            AllocTraits::destroy(alloc_, &(static_cast<Node*>(node)->value));
            BaseNode* next_node = node->next;
            AllocTraits::deallocate(alloc_, static_cast<Node*>(node), 1);
            node = next_node;
        }
    }

    template <typename... Args>
    [[maybe_unused]] iterator insert(iterator pos, Args&&... args) {
        Node* new_node = create_node(std::forward<Args>(args)...);
        rebind_nodes(pos, new_node);
        ++sz_;
        return iterator {new_node};
    }

    void push_back(const T& val) {
        insert(end(), val);
    }

    void push_front(const T& val) {
        insert(begin(), val);
    }

    iterator erase(const_iterator first, const_iterator last) {
        iterator prev = first;
        --prev;

        iterator it = first;
        for (;it != last;) {
            it = delete_node(it);
        }
        it = delete_node(it);

        bind_nodes(prev, it);

        return it;
    }

    size_type size() const {
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

    iterator delete_node(iterator pos) {
        iterator ret = pos.ptr_->next;
        
        AllocTraits::destroy(alloc_, &(static_cast<Node*>(pos.ptr_)->value));
        AllocTraits::deallocate(alloc_, static_cast<Node*>(pos.ptr_), 1);
        --sz_;

        return ret;
    }

    void bind_nodes(iterator lhs, iterator rhs) {
        lhs.ptr_->next = rhs.ptr_;
        rhs.ptr_->prev = lhs.ptr_;
    }

    void rebind_nodes(iterator pos, BaseNode* new_node) {
        BaseNode* prev_node = pos.ptr_->prev;
        
        prev_node->next = new_node;
        new_node->prev = prev_node;
        new_node->next = pos.ptr_;
        pos.ptr_->prev = new_node;
    }
    

   
};