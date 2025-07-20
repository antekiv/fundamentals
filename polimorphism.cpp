#include <iostream>

// Idea of virtual functions
// polimorphic·type - the type which has at most one virtual function or inherited the last one.
struct Base {
    // always need a virtual destructor
    virtual ~Base() { std::cout << "~Base" << std::endl;}
    virtual void f(){
        std::cout << 1 << std::endl;
    }
};

struct Derived : Base {
    void f() override { // or final - compiler can optimaze it 
        std::cout << 2 << std::endl;
    }
    ~Derived(){std::cout << "~Derived\n";}
};

// context dependant key word
// int override = 5; OK
// Derived d;
// d.Base::f(); // won't execute virtual. Compile time

// Abstract classes(interface) and pure virtual functions

// Abstract class (Has at most one pure virtual method).
// Can not create an object of this class:
struct Shape {
    // pure virtual function
    virtual double area() const = 0;
    virtual ~Shape() = default;
};

double Shape::area() const {
    return 0.0;
}

struct Square : Shape {
    double a_;
    Square(double a) : a_(a) {}
    double area() const override {
        return a_ * a_;
    }
};

int main()
{
    Square s(5);
    std::cout << "Square: " << s.area() << " Base: " << s.Shape::area()
        << std::endl;
}
