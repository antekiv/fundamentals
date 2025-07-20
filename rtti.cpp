#include <iostream>

// RTTI (RunTime Type Information) and dynamic cast

//
//     Granny    | downcast  ^ upcast        horizontal cast
//     /    \    | (g -> s)  | (s->m)        (m <--> d)
//   Mom    Dad  |           | (m->g)  <--->
//     \    /    |           |
//      Son      ↓           |
//
// dynamic_cast:
//
// upcast - OK
// downcast - OK (for polimorphic types)
// horizontcast - OK

// static_cast:
// 
// upcast - OK
// downcast - UB
// horizontalcast - CE

// reintrept_cast:
// 
// upcast - UB
// downcast - UB
// horizontalcast - UB

/*
struct Base {
    virtual void f() {}
    virtual ~Base() = default;
};

struct Derived : Base {
    void f() override {}
};

int main()
{
    Derived d;
    Base& b = d;
    
    // typeid returns typeinfo - real type under the ref
    // works for other types
    // Mangling name
    // c++filt -t 7Derived
    std::cout << typeid(b).name() << std::endl;
   
    // if with initialization
    // cast to ref will throw an exception
    // possible polimorphic type -> void*. revert isn't possible
    // can always do upcast (Derived -> Base) works as static cast for all types
    // cast in horizont or downcast (for polimorphic types)
    // can also do polimorphic type with virtual des-or
    if (Derived* pd = dynamic_cast<Derived*>(&b); pd) {
        // Dyncamic cast works only for types with virtual functions
        // do something. Can be used for compare shapes
    }
}
*/

// Memory layout of polimorphic objects

// sizeof(Base) = 16
/*
struct Base {
    virtual void f() {}
    void h() {}
    int x = 0; 
};

struct Derived : Base {
    void f() override {}
    virtual void g(){};
    int y;
};
*/

// vtable           Base&
// Base             [ptr][x][pend] - stack
//                      \
// [&Base::type_info][&Base::f]
//        |
//   [type_name] (const char*)

// vtable             Derived&
// Derived            [ptr][x][y]
//                       \
// [&Derived::type_info][&Derived::f][&Derived::g]   
//         |  
//     [type_name] (const char*)

// Granny   (int g, void h())
//   |
//  Mom     (int m, virtual void f())
//   |
//  Son     (int s, void f() override)
//
// [ptr][g][m][s]
//  \    \
//   s/m  g
//
// static/dinamic_cast: s -> g. Revert for dynamic_cast isn't possible - object isn't polimorphic. Static_cast is possible 

// Memory layout of polimorfic oblects in case multiple inheritance

// Virtual functions non virtual inheritance
//
//  Granny (virtual void f())
//     |   (int g)
//      \           Granny (the same)
//       \             /
//       Mom (m)      Dad (d)
//         \         /
//            Son (s)
//
//  [ptr][g][m][ptr][g][d][s][..]
// 22. ???

int main()
{
    Base b;
    std::cout << sizeof(b) << std::endl;
}
