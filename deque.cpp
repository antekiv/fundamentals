#include <array>
#include <limits>
#include <vector>
#include <type_traits>
#include <iostream>


template <typename T, typename Alloc = std::allocator<T>>
class Deque {
    static constexpr size_t BUF_SIZE = 32;

template <bool IsConst>
    class base_iterator {
    public:
        using pointer_type = std::conditional_t<IsConst, const T*, T*>;
        using reference_type = std::conditional_t<IsConst, const T&, T&>;
        using value_type = T;
    private:
        T* ptr_        = nullptr;
        size_t buf_ind_ = 0;
        
    public:
        base_iterator(T* ptr = nullptr, size_t buf_ind = 0)
                : ptr_(ptr)
                , buf_ind_(buf_ind) {}

        base_iterator(const base_iterator&) = default;
        base_iterator& operator=(const base_iterator&) = default;

        bool operator==(const base_iterator& other) const {
            return std::tie(ptr_, buf_ind_) == std::tie(other.ptr_, other.buf_ind_);
        }
        
        reference_type operator*() const {return ptr_[buf_ind_];}
        pointer_type operator->() const  {return &ptr_[buf_ind_];}

        base_iterator& operator++() {
            iter_inc_();
            return *this;
        }
        base_iterator& operator++(int) {
            base_iterator copy = *this;
            iter_inc_();
            return copy;
        }

        base_iterator& operator--() {
            iter_dec_();
            return *this;
        }
        base_iterator& operator--(int) {
            base_iterator copy = *this;
            iter_dec_();
            return copy;
        }

    private:
        void iter_inc_() {     
            if (buf_ind_ != BUF_SIZE) {
                ++buf_ind_;
            } else {
                ++ptr_;
                buf_ind_ = 0;
            }
        }
        void iter_dec_() {
            if (buf_ind_ != std::numeric_limits<size_t>::max()) {
                --buf_ind_;
            } else {
                --ptr_;
                buf_ind_ = BUF_SIZE - 1;
            }
        }
    };

public:
    using const_iterator = base_iterator<true>;
    using iterator = base_iterator<false>;
    
    iterator begin() {
        return begin_;
    }

    // const?
    iterator end() {
        return end_;
    }

    const_iterator begin() const {
        return begin_;
    }

    const_iterator end() const {
        return end_;
    }

    // TODO: add reverse iterator

public:
    using AllocTraits = std::allocator_traits<Alloc>;

    Deque() {}
    Deque(size_t size, T def_value = T()) {}
    ~Deque() {
        for (; begin_ != end_; ++begin_ ) {
            AllocTraits::destroy(alloc_, &(*begin_));
        }

        for (auto& buf_ptr: buffers_)
            AllocTraits::deallocate(alloc_, buf_ptr, BUF_SIZE);
    }

    template <typename... Args>
    void emplace_back(Args&&... value) {
        if (end_ == deq_end())
            back_resize(); 

        AllocTraits::construct(alloc_, &(*end_), std::forward<decltype(value)>(value)...);
        ++end_;
    }

    T& operator[](size_t ind) const { 
        return (buffers_[0])[0];
    }

    T& operator[](size_t ind) { 
        return (buffers_[0])[0];
    }

    T& at(size_t ind) {
        return (buffers_[0])[0];
    }

    size_t size() const {
        return size_;
    }

private:
    std::vector<T*> buffers_;
    iterator begin_;
    iterator end_;
    size_t size_ = 0;
    [[no_unique_address]] Alloc alloc_ = Alloc();

private:
    iterator deq_begin() {
        if (buffers_.empty())
            return {};

        return {buffers_[0], 0};
    }
    iterator deq_end() {
        if (buffers_.empty())
            return {};

        return {buffers_.back(), BUF_SIZE};
    }

    void back_resize() {
        T* new_arr = AllocTraits::allocate(alloc_, BUF_SIZE);
        buffers_.push_back(new_arr);

        // TODO: rethink
        begin_ = deq_begin();
        end_ = (end() == iterator{})
            ? deq_begin()
            : --deq_end();
    }
};

// need to remember (i1, j1) for begin, (i2, j2) for end