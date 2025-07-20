#include <iostream>
#include <memory>

// Custom deleter. Can be used not only for deleting.
// You can shut down or close any connection here 
template <typename T>
struct my_deleter {
    void operator()(T* ptr) const {
        delete ptr;
    }
};

template <typename T, typename Deleter = std::default_delete<T>>
class unique_ptr {
    T* ptr_;
    // until c++20 - EBO - private derive 
    [[no_unique_address]] Deleter del_;
public:
    unique_ptr(T* ptr, Deleter del = std::default_delete<T>()) : ptr_(ptr), del_(del) {}
    ~unique_ptr() {
        del_(this->ptr_);
    }

    unique_ptr(const unique_ptr&) = delete;
    unique_ptr& operator=(const unique_ptr&) = delete;

    unique_ptr(unique_ptr&& other)
        : ptr_(other.ptr_)
        , del_(std::move(other.del_)) {
        other.ptr_ = nullptr;
    }

    unique_ptr& operator=(unique_ptr&& other) {
        if (this != other.ptr_) {
            del_(ptr_);
            ptr_ = other.ptr_;
            del_ = std::move(other.del_);

            other.ptr_ = nullptr;
        }
        return *this;
    }

    T& operator*() const {
        return *ptr_;
    }

    T* operator->() const {
        return ptr_;
    }

    T* get() const {
        return ptr_;
    }

    T* release() {
        T* tmp = nullptr;
        std::swap(tmp, ptr_);
        return tmp;
    }
};

template <typename T, typename... Args>
unique_ptr<T> make_unique(Args&&... args) {
    return unique_ptr<T>(new T(std::forward<Args>(args)...));
}

int main()
{
    unique_ptr<int> ptr = make_unique<int>(54);
}