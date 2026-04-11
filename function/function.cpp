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
    
    alignas (max_align_t) char buffer[BUFFER_SIZE];
    using invoke_ptr_t = Ret(*)(void*, Args...);
    using destroy_ptr_t = void(*)(void*);

    void* fptr_;
    invoke_ptr_t invoke_ptr_;
    destroy_ptr_t destroy_ptr_;

public:
    template <typename F>
    static Ret invoker(F* fptr, Args... args) {
        return (*fptr)(std::forward<Args>(args)...);
    }

    template <typename F>
    static void destroyer(F* fptr) {
        if constexpr (sizeof(F) > BUFFER_SIZE) {
            delete fptr;
        } else {
            fptr->~F();
        }
    }

    
    template <typename F>
    function(const F& func)
        : invoke_ptr_(reinterpret_cast<invoke_ptr_t>(&invoker<F>))
        , destroy_ptr_(reinterpret_cast<destroy_ptr_t>(&destroyer<F>))
    {
        if constexpr (sizeof(F) > BUFFER_SIZE) {
            fptr_ = new F(func);
        } else {
            new (buffer) F(func);
            fptr_ = buffer;
        }
    }

    ~function() {
        destroy_ptr_(fptr_); 
    }

    Ret operator()(Args... args) const {
        return invoke_ptr_(fptr_, std::forward<Args>(args)...);
        //return fptr->call(std::forward<Args>(args)...);
    }
};