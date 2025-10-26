// List realization

#include <memory>
#include <iostream>
#include <iterator>

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
        friend void List<T, Allocator>::rebind_nodes(base_iterator<true>, BaseNode*);
        friend void List<T, Allocator>::bind_nodes(base_iterator<true>, base_iterator<true>);
        friend base_iterator<false> List<T, Allocator>::erase(base_iterator<true>);
        friend base_iterator<true> List<T, Allocator>::delete_node(base_iterator<true>);

        template<bool>
        friend class base_iterator;
    public:
    
        using pointer_type = std::conditional_t<IsConst, const T*, T*>;
        using reference_type = std::conditional_t<IsConst, const T&, T&>;
        using difference_type = std::ptrdiff_t;
        using value_type = T;
        using iterator_category = std::bidirectional_iterator_tag;
        //using iterator_type = base_iterator;
    private:
        BaseNode* ptr_;
        
    public:
        base_iterator(const BaseNode* ptr): ptr_(ptr) {}
        base_iterator<true>(const base_iterator<false>& it) : ptr_(it.ptr_) {}

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

    const_iterator cbegin() {
        return const_iterator{fake_node_.next};
    }

    const_iterator cend() {
        return const_iterator{end()};
    }

    // reverse_iterators
    reverse_iterator rbegin() {
        return reverse_iterator{end()};
    }

    reverse_iterator rend() {
        return reverse_iterator{begin()};
    }

    const_reverse_iterator rbegin() const {
        return const_reverse_iterator{end()};
    }

    const_reverse_iterator rend() const {
        return const_reverse_iterator{begin()};
    }

    const_reverse_iterator crbegin() {
        return const_reverse_iterator{cend()};
    }

    const_reverse_iterator crend() {
        return const_reverse_iterator{cbegin()};
    }

public:
    using AllocTraits = std::allocator_traits<decltype(alloc_)>;
    explicit List(const Allocator& alloc = Allocator()) 
        : fake_node_{&fake_node_, &fake_node_}
        , sz_{0}
        , alloc_(alloc) {}

    List(const List& other) {
        
        std::cout << other.size() << std::endl;
        //List temp(other.alloc_);
        std::cout << "4\n";
        for (auto el : other) {
          //  temp.push_back(el);
        }

        std::cout << "LI\n";

        /*
        std::swap(this->fake_node_, temp.fake_node_);
        std::swap(this->sz_, temp.sz_);
        this->alloc_ = temp.alloc_;
        std::cout << "LISTTT\n";
        */
    }

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
    [[maybe_unused]] iterator insert(const_iterator pos, Args&&... args) {
        Node* new_node = create_node(std::forward<Args>(args)...);
        rebind_nodes(pos, new_node);
        ++sz_;
        return iterator {new_node};
    }

    void push_back(const T& val) {
        insert(cend(), val);
    }

    void  push_front(const T& val) {
        insert(cbegin(), val);
    }

    [[maybe_unused]] iterator erase(const_iterator pos) {
        const_iterator prev = pos - 1;
        pos = delete_node(pos);
        bind_nodes(prev, pos);

        return iterator(pos.ptr_);
    }

    void pop_front() {
        erase(cbegin());
    }

    void pop_back() {
        erase(cend() - 1);
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

    const_iterator delete_node(const_iterator pos) {
        const_iterator ret = pos + 1;
        
        AllocTraits::destroy(alloc_, &(static_cast<Node*>(pos.ptr_)->value));
        AllocTraits::deallocate(alloc_, static_cast<Node*>(pos.ptr_), 1);
        --sz_;

        return ret;
    }

    void bind_nodes(const_iterator lhs, const_iterator rhs) {
        lhs.ptr_->next = rhs.ptr_;
        rhs.ptr_->prev = lhs.ptr_;
    }

    void rebind_nodes(const_iterator pos, BaseNode* new_node) {        
        BaseNode* prev_node = pos.ptr_->prev;

        prev_node->next = new_node;
        new_node->prev = prev_node;
        new_node->next = pos.ptr_;
        pos.ptr_->prev = new_node;
    }
};