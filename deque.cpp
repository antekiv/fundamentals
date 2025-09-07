#include <array>
#include <limits>
#include <vector>
#include <type_traits>
#include <iostream>
#include <utility>

template <typename T, typename Alloc = std::allocator<T>>
class Deque {
    static constexpr size_t BUF_SIZE = 32;
    static constexpr size_t DEQ_BEG = BUF_SIZE / 2;

    template <bool IsConst>
    class base_iterator {
    public:
        using pointer_type = std::conditional_t<IsConst, const T*, T*>;
        using reference_type = std::conditional_t<IsConst, const T&, T&>;
        using value_type = T;
    private:
        template <typename Self>
        friend decltype(auto) Deque::operator[](this Self&&, size_t);

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

        base_iterator<true> to_const() const {
            return base_iterator<true>(this->ptr_, this->buf_ind_);
        }

        T** get_ptr() const {
            return ptr_;
        }

        size_t get_buf_ind() const {
            return buf_ind_;
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
            //std::cout << "iter_dec_: " << *ptr_ << " -> " << buf_ind_ << std::endl;
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

    iterator end() {
        return end_;
    }

    const_iterator begin() const {
        return begin_.to_const();
    }

    const_iterator end() const {
        return end_.to_const();
    }

    // TODO: add reverse iterator

public:
    using AllocTraits = std::allocator_traits<Alloc>;

    Deque() {}
    Deque(const Deque& other)
            : alloc_(other.alloc_) {
        for (const auto& el : other) {
            this->emplace_back(el);
        }
    }
    Deque(size_t size, T def_value = T()) {
        Deque temp;
        for (size_t i = 0; i < size; ++i) {
            temp.emplace_back(def_value);
        }
        swap(temp);
    }

    Deque& operator=(const Deque& other) {
        if (this == &other)
            return *this;

        Deque temp;
        for (const auto& el : other) {
            temp.emplace_back(el);
        }
        swap(temp);
        return *this;
    }

    ~Deque() {
        for (; begin_ != end_; ++begin_ )
            AllocTraits::destroy(alloc_, &(*begin_));

        for (auto& buf_ptr: buffers_)
            AllocTraits::deallocate(alloc_, buf_ptr, BUF_SIZE);
    }

    template <typename... Args>
    void emplace_back(Args&&... value) {
        if (end_ == deq_end())
            resize_back(); 

        AllocTraits::construct(alloc_, &(*end_), std::forward<decltype(value)>(value)...);

        ++end_;
        ++size_;
    }
    template <typename... Args>
    void emplace_front(Args&&... value) {
        if (!buffers_.empty())
            --begin_;

        if (begin_ == deq_null_begin())
            resize_front(); 

        AllocTraits::construct(alloc_, &(*begin_), std::forward<decltype(value)>(value)...);
        ++size_;
    }

    template <typename U>
    void push_back(U&& value) {
        emplace_back(std::forward<U>(value));
    }
    template <typename U>
    void push_front(U&& value) {
        emplace_front(std::forward<U>(value));
    }
    
    void pop_back() {
        --end_;
        --size_;
    }
    void pop_front() {
        ++begin_;
        --size_;
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

        //std::cout << "operator[]: " << *ptr << " -> " << buf_ind << std::endl;
        if constexpr (std::is_const_v<std::remove_reference_t<Self>>) {
            return static_cast<const std::remove_reference_t<T>&>((*ptr)[buf_ind]);
        } else {
            return static_cast<std::remove_reference_t<T>&>((*ptr)[buf_ind]);
        }
    }

    decltype(auto) at(this auto&& self, size_t ind) {
        if (self.size_ <= ind)
            throw std::out_of_range("");

        return self[ind];
    }

    size_t size() const {
        return size_;
    }

    void swap(Deque& other)
    {
        std::swap(buffers_, other.buffers_);
        std::swap(size_, other.size_);
        std::swap(alloc_, other.alloc_);
        std::swap(begin_, other.begin_);
        std::swap(end_, other.end_);
    }

private:
    std::vector<T*> buffers_;
    size_t size_ = 0;
    [[no_unique_address]] Alloc alloc_ = Alloc();
    iterator begin_;
    iterator end_;

private:
    iterator deq_begin() {
        if (buffers_.empty())
            return {};

        return {&buffers_[1], DEQ_BEG};
    }
    iterator deq_null_begin() {
        if (buffers_.empty())
            return {};

        return {&buffers_[0], BUF_SIZE - 1};
    }

    iterator deq_end() {
        if (buffers_.empty())
            return {};

        return {&buffers_.back(), 0};
    }

    void initialize_buffer() {
        if (buffers_.size())
            return;
        
        T* new_buff = AllocTraits::allocate(alloc_, BUF_SIZE);
        
        buffers_.reserve(3);
        buffers_.push_back(nullptr);
        buffers_.push_back(new_buff);
        buffers_.push_back(nullptr);

        begin_ = deq_begin();
        end_ = deq_begin();
    }

    std::pair<size_t, size_t> find_inds(const iterator& it){
        if (it.get_ptr() == nullptr)
            return {0, 0};

        size_t index_ptr = 0;
        for (const auto& ptr : buffers_) {
            if (ptr == *it.get_ptr())
                break;
            ++index_ptr;
        }  
        return {index_ptr, it.get_buf_ind()};
    }


    void resize_back() {
        if (buffers_.empty()) {
            initialize_buffer();
        } else {
            T* new_buff = AllocTraits::allocate(alloc_, BUF_SIZE);
            buffers_.back() = new_buff;
            buffers_.push_back(nullptr);

            // structure binding
            auto [first, second] = find_inds(begin_);

            begin_ = iterator{&buffers_[first], second};
            end_ = iterator{&buffers_[buffers_.size() - 2], 0};
        }
    }

    void resize_front() {
        if (buffers_.empty()) {
            initialize_buffer();
        } else {
            std::vector<T*> new_buffers(buffers_.size() + 1, nullptr);
            std::copy(buffers_.begin(), buffers_.end(), std::next(new_buffers.begin()));
            
            T* new_buff = AllocTraits::allocate(alloc_, BUF_SIZE);
            new_buffers[1] = new_buff;

            auto [first, second]  = find_inds(end_);

            buffers_ = std::move(new_buffers);
            begin_ = iterator{&buffers_[1], BUF_SIZE - 1};
            end_ = iterator{&buffers_[first + 1], second};
        }
    }
};

// need to remember (i1, j1) for begin, (i2, j2) for end