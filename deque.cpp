#include <array>
#include <limits>
#include <vector>
#include <type_traits>
#include <iostream>
#include <utility>


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
        template <typename Self>
        friend decltype(auto) Deque::operator[](this Self&&, size_t);
        //friend T& Deque::operator[](size_t);
        //friend const T& Deque::operator[](size_t) const;

        T** ptr_        = nullptr;
        size_t buf_ind_ = 0;
        
    public:
        base_iterator(T** ptr = nullptr, size_t buf_ind = 0)
                : ptr_(ptr)
                , buf_ind_(buf_ind) {}

        base_iterator(const base_iterator&) = default;
        base_iterator& operator=(const base_iterator&) = default;

        bool operator==(const base_iterator& other) const {
            //std::cout << "(" << ptr_ << ", " << buf_ind_ << ") == (" << other.ptr_ << ", " << other.buf_ind_ << ")" << std::endl;
            return std::tie(ptr_, buf_ind_) == std::tie(other.ptr_, other.buf_ind_);
        }
        
        reference_type operator*() const {return (*ptr_)[buf_ind_];}
        pointer_type operator->() const  {return &(*ptr_)[buf_ind_];}

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
            ++buf_ind_;
            if (buf_ind_ == BUF_SIZE) {
                ++ptr_;
                buf_ind_ = 0;
            } 
        }
        void iter_dec_() {
            --buf_ind_;

            if (buf_ind_ == std::numeric_limits<size_t>::max()) {
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
        for (; begin_ != end_; ++begin_ )
            AllocTraits::destroy(alloc_, &(*begin_));

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

    template<typename Self>
    decltype(auto) operator[](this Self&& self, size_t ind) {
        auto ptr = self.begin_.ptr_; 
        size_t buf_ind;

        if (self.begin_.buf_ind_ + ind < BUF_SIZE) {
            buf_ind = self.begin_.buf_ind_ + ind;
        } else {
            auto common = ind - BUF_SIZE + self.begin_.buf_ind_;
            buf_ind = common % BUF_SIZE;
            auto forward = 1 + ((common) / BUF_SIZE);
            ptr += forward;
        }

        if constexpr (std::is_const_v<std::remove_reference_t<Self>>) {
            return static_cast<const std::remove_reference_t<T>&>((*ptr)[buf_ind]);
        } else {
            return static_cast<std::remove_reference_t<T>&>((*ptr)[buf_ind]);
        }
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

        return {&buffers_[0], 0};
    }
    iterator deq_end() {
        if (buffers_.empty())
            return {};

        return {&buffers_.back(), 0};
    }

    void back_resize() {
        T* new_arr = AllocTraits::allocate(alloc_, BUF_SIZE);

        if (buffers_.empty()) {
            buffers_.push_back(new_arr);
        } else {
            buffers_.back() = new_arr;
        }
        buffers_.push_back(nullptr);

        // TODO: rethink
        begin_ = deq_begin();
        end_ = (end() == iterator{})
            ? deq_begin()
            // last_buf_begin
            : iterator{&buffers_[buffers_.size() - 2], 0};
    }
};

// need to remember (i1, j1) for begin, (i2, j2) for end