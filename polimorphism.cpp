#include <iostream>

// Idea of virtual functions
// polimorphic·type - the type which has at most one virtual function or inherited the last one.
struct Base {
    // always need a virtual destructor
    ~Base() { std::cout << "~Base" << std::endl;}
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

#include <vector>
#include <algorithm>
// Observer

// pure virtual
struct IObserver {
    virtual void update(const std::string& news) = 0;
    virtual ~IObserver() = default;
};

struct NewsAgency {
    void Subscribe(IObserver* obs) { 
        observers.push_back(obs);
    }
    void Unsubscribe(IObserver* obs) { 
        std::remove(observers.begin(), observers.end(), obs), observers.end();
    }
    void PublishNews(const std::string& news) {
        notify(news);
    }

    ~NewsAgency() {
        for (const auto& obs : observers) {
            delete obs;
        }
    }

private:
    void notify(const std::string& news) {
        for (const auto& obs : observers) {
            obs->update(news);
        }
    }
    
private:
    std::vector<IObserver*> observers;
};

struct TVViwer : IObserver {
    void update(const std::string& news) override {
        std::cout << "TVViewer: " << news << std::endl;
    }
};

struct YoutubeViwer : IObserver {
    void update(const std::string& news) override {
        std::cout << "YoutubeViwer: " << news << std::endl;
    }
};

struct RadioViwer : IObserver {
    void update(const std::string& news) override {
        std::cout << "RadioViwer: " << news << std::endl;
    }
};

int main()
{
    NewsAgency na;
    na.Subscribe(new TVViwer());
    na.Subscribe(new YoutubeViwer());
    na.Subscribe(new RadioViwer());
    na.PublishNews("Hello Kitty!");

}
