#include <iostream>
#include <memory>

template <typename T>
class function;

template <typename Ret, typename... Args>
class function<Ret(Args...)> {
    struct Base {
        virtual Ret call(Args...) = 0;
        virtual ~Base() = default;
    };

    template <typename F>
    struct Derived : Base {
        F f;
        ~Derived() override = default;
        Derived(const F& f) : f(f) {}
        Derived(F&& f) : f(std::move(f)) {}
        
        Ret call(Args... args) override {

            if constexpr (std::is_member_function_pointer_v<F>) {
                // TODO: call with correct syntax
            } else if constexpr (std::is_member_object_pointer_v<F>) {
                // TODO
            } else {
                return f(std::forward<Args>(args)...);
            }
        }
    };

   
private:
    static const size_t BUFFER_SIZE = 16;
    Base* fptr;
    
    alignas (max_align_t) char buffer[BUFFER_SIZE];
    using invoke_ptr_t = Ret(*)(void*, Args...);
    invoke_ptr_t invoke_ptr;

public:
    template <typename F>
    static Ret invoker(F* fptr, Args... args) {
        // std::invoke
        // pointer to member?

        return (*fptr)(std::forward<Args>(args)...);
    }

    
    template <typename F>
    function(const F& func)
        : invoke_ptr(reinterpret_cast<invoke_ptr_t>(&invoker<F>))
        , destroy_ptr(reinterpret_cast<invoke_ptr_t>(&invoker<F>))
    {
        if constexpr (sizeof(F) > BUFFER_SIZE) {
            fptr = new F(func);
        } else {
            new (buffer) F(func);
            fptr = buffer;
        }
    }

    ~function() {
        delete fptr; 
    }

    Ret operator()(Args... args) const {
        return fptr->call(std::forward<Args>(args)...);
    }
};