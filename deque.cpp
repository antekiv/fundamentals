#include <array>
#include <limits>
#include <vector>



template <typename T, typename Allocator = std::allocator<T>>
class Deque {
    static constexpr size_t BUF_SIZE = 32;
    
    std::vector<std::array<T, BUF_SIZE>*> buffers_;
    size_t begin_ptr_ind_ = 0;
    size_t begin_buf_ind_ = 0;
    size_t size_;

private:
template <bool IsConst>
    class base_iterator {
    public:
        using pointer_type = std::conditional_t<IsConst, const T*, T*>;
        using reference_type = std::conditional_t<IsConst, const T&, T&>;
        using value_type = T;
    private:
        pointer_type ptr_;
        size_t buf_ind_;
        
    public:
        base_iterator(T* ptr = nullptr, size_t buf_ind = 0)
                : ptr_(ptr)
                , buf_ind_(buf_ind) {}

        base_iterator(const base_iterator&) = default;
        base_iterator& operator=(const base_iterator&) = default;

        bool operator==(const base_iterator& other) const {
            return std::tie(ptr_, buf_ind_) == std::tie(other.ptr_, other.buf_ind_);
        }
        
        reference_type operator*() const {return (*ptr_)[buf_ind_];};
        pointer_type operator->() const {return &(*ptr_)[buf_ind_];}

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
        return {&buffers_[begin_ptr_ind_], begin_buf_ind_};
    }

    iterator end() {
        // decrement to end() should get the last element
        int end_ptr_ind = begin_ptr_ind_; 
        int end_buf_ind;

        if (begin_buf_ind_ + size_ <= BUF_SIZE) {
            end_buf_ind = begin_buf_ind_ + size_;

        } else {
            auto common = size_ - BUF_SIZE + begin_buf_ind_;
            end_buf_ind = common % BUF_SIZE;

            end_ptr_ind += 1 + (common - 1) / BUF_SIZE;

            if (end_buf_ind == 0)
                end_buf_ind = BUF_SIZE;
        }
        return {&buffers_[end_ptr_ind], end_buf_ind};
    }
/*
    const_iterator begin() const {
        return {arr_};
    }

    const_iterator end() const {
        return {arr_ + sz_};
    }

    const_iterator cbegin() const {
        return {arr_};
    }

    const_iterator cend() const {
        return {arr_ + sz_};
    }
        */
public:
    Deque() {}
    Deque(size_t size, T def_value = T()) {}

    T& operator[](size_t ind) const { 
        return (*buffers_[0])[0];
    }

    T& operator[](size_t ind) { 
        return (*buffers_[0])[0];
    }

    T& at(size_t ind) {
        //if ()
    }

    size_t size() const {
        return size_;
    }

private:


};

// need to remember (i1, j1) for begin, (i2, j2) for end