#include <iostream>
#include <concepts>

// Концепт для проверки наличия метода act
template <typename T>
concept HasActMethod = requires(T obj) {
    { obj.act() }; // Проверка на наличие метода act
};
class B {
public:
    void doSomething() {
        std::cout << "B::doSomething called" << std::endl;
    }
};
// Пример класса с методом act
class A:public B {
public:
    void act() {
        std::cout << "A::act called" << std::endl;
    }
};

// Пример класса без метода act


// Функция для проверки наличия метода act у объекта
template <HasActMethod T>
void checkActMethod(T& obj) {
    obj.act();
}

template <typename T>
void checkActMethod(T& obj) {
    std::cout << "Object does not have an act method." << std::endl;
}

int main() {
    A a;
    B b;
	B* c;
	c = new A;

    checkActMethod(a); // Вывод: A::act called
    checkActMethod(b); // Вывод: Object does not have an act method.
	checkActMethod(c);
    return 0;
}

