#include <iostream>

struct A {}; // 1 byte

// EBO empty base optimization
struct B : A {int x;};// 4 byte




// Multiple inheritance

// struct Mom { int m;};
// struct Dad { int d;};
// struct Son : Mom, Dad {int d;};
//
// | m | d | s | = 12 bytes
// downcast may change the address:
// Dad* p = &s;
// static_cast<Dad&>(s)
//
// struct Mom { void f(){}};
// struct Dad1 { void f(){}};
// in Son methods f from Mom and Son are equal in overloading
// if we will execute one of equal methods - CE
// s.Dad::f(); is OK
// struct Son : Mom, Dad {};


// Diamond problem
//
//    Granny (int g)      Granny  Granny
//    /     \                |      |
//  Mom (m) Dad (d)    -->  Mom    Dad
//     \   /                   \   /
//      Son (s)                 Son
//
// | g | m | g | d | s | = 20 bytes 
//
// S.g; CE
// S.Mom::g; OK
//
// Cast:
// Granny& g = s; // CE ambigious cast
// static_cast<Mom::Granny&>(s); // CE - types are equal
// Mom& m = s;
// Granny& g = m; // OK
//
// Mom -> Dad cast:
// Mom& m = s;
// static_cast<Dad&>(m); // CE
// 
// Granny -> Son - CE - several Grannies
// Granny -> Mom; Mom -> Son - OK

// Inaccessable base class:
//
//   Granny
//     |
//    Dad Granny (int g)
//      \   /
//       Son
//
// s.Granny::g; - // 2 grannies
 
// Pointers to methods:
//
// Mom {void f(){}}  Dad {void f(){}}
//   \                /
//    Son {void f(){}}   
// 
// | &f | shift |  pointer to method. Fields Are Shifted!!!!
// void (Mom::* p)() = &Mom::f; // 16 bytes 
// void·(Son2::*·ps)()·=·p;
// (s.*ps)();
//
// f(this*, ...) {}
// 
// f| m f| d f| s |


// Virtual inheritance
//
//     Granny (int g)      struct Mom : public virtual Granny{};
//    /     \              struct Dad : public virtual Dad{};
//  Mom(m) Dad(d)          struct Son : Mom, Dad{};
//     \   / 
//     Son(s)
//
// Son s;
// s.g; // OK
// Granny& g = s; // OK
//
// | shift_to_g | m |...pending| shift_to_g | d | s | g |.. pedding || = 40 bytes!! 
// vtbls: Mom[12], Dad[12], Son[32] - shift relatively the beginning of the object AND TOPOFFSET(place in object relatively beginning) (keeps in static memory; there are also global, const, local static)
// static_cast in downcast - CE (G -> S) - don't have vtable  
// ????
// 
struct Granny { int g;};

struct Mom : virtual Granny{
    void f(){std::cout << "Mom\n";}
    int m;
};
struct Dad : virtual Granny{
    int d;
    void f(){std::cout << "Dad\n";}
};
struct Son : Mom, Dad {
    int s;
    void f(){std::cout << "Son\n";}
};


int main()
{
    Son s;
    std::cout << sizeof(s) << std::endl;
}
