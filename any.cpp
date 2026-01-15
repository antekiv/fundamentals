#include <iostream>
// type eraasure 
/*
class any {
    struct Base {
        virtual Base* getCopy() const = 0;
         virtual ~Base() = default;
    };

    template <typename T>
    struct Derived : Base {
        T value_;
        Derived(const T& value) : value_(value) {}
        Derived(T&& value) : value_(std::move(value)) {}
        Base* getCopy() const override {
            return new Derived(value_);
        }
    };

    Base* ptr_;
    
    template <typename T>
    friend T& any_cast(any& a);

public:
    template <typename T>
    any(const T& value) : ptr_(new Derived<T>(value)){

    }
    any(const any& other) : ptr_(other.ptr_->getCopy()) {}

    ~any() {
        delete ptr_;
    }
};

template <typename T>
T& any_cast(any& a) {
    auto* p = dynamic_cast<any::Derived<std::remove_reference_t<T>>*>(a.ptr_);

    if (!p)
        throw std::runtime_error("bad any cast");

    return p->value_;
}
*/


class any {
    struct Base {
         virtual ~Base() = 0;
    };
    

    template <typename T>
    struct Derived : Base {
        T value_;
        Derived(const T& value) : value_(value) {}
        Derived(T&& value) : value_(std::move(value)) {}
        ~Derived() = default; //override { std::cout << "~Derived" << std::endl;}
    };

    Base* ptr_;
    
public:
    template <typename T>
    any(const T& value) : ptr_(new Derived<T>(value)) {}
    
    ~any() {
        delete ptr_;
    }
};

inline any::Base::~Base() {}

int main() {
    
    any a1(4);
    any a2(std::string("string"));
    any a3(3.14);

    return 0;
}