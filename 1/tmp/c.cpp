#include <iostream>
#include <coroutine>
#include <chrono>
#include <random>
#include <vector>
#include <deque>
#include <thread>  // Для std::this_thread::sleep_for

// Структура для ожидания асинхронного времени
struct TimerAwaiter {
    std::chrono::milliseconds duration;
    TimerAwaiter(std::chrono::milliseconds d) : duration(d) {}

    bool await_ready() const noexcept { 
        return false; // всегда приостанавливается
    }

    void await_suspend(std::coroutine_handle<> handle) const noexcept {
        // Устанавливаем отложенный запуск корутины
        std::thread([handle, this]() {
            std::this_thread::sleep_for(this->duration);
            handle.resume();  // Резюмируем выполнение корутины после задержки
        }).detach();
    }

    void await_resume() const noexcept {}
};

// Функция для эмуляции асинхронной задержки
TimerAwaiter sleep_for(std::chrono::milliseconds duration) {
    return TimerAwaiter(duration);
}

// Структура для корутины
struct Task {
    struct promise_type;
    using handle_type = std::coroutine_handle<promise_type>;

    handle_type coro_handle;

    Task(handle_type h) : coro_handle(h) {}
    ~Task() {
        if (coro_handle) coro_handle.destroy();
    }

    Task(Task&& other) noexcept : coro_handle(other.coro_handle) {
        other.coro_handle = nullptr;
    }

    Task& operator=(Task&& other) noexcept {
        if (this != &other) {
            if (coro_handle) coro_handle.destroy();
            coro_handle = other.coro_handle;
            other.coro_handle = nullptr;
        }
        return *this;
    }

    bool done() const {
        return coro_handle.done();
    }

    void resume() {
        if (coro_handle && !coro_handle.done()) {
            coro_handle.resume();
        }
    }

    struct promise_type {
        auto get_return_object() {
            return Task{handle_type::from_promise(*this)};
        }
        std::suspend_always initial_suspend() { return {}; }  // Приостанавливаем выполнение сразу
        std::suspend_always final_suspend() noexcept { return {}; }  // Завершаем корректно
        void return_void() {}
        void unhandled_exception() { std::terminate(); }
    };
};

// Класс A с полем value
class A {
public:
    int value;

    A() : value(0) {}

    // Асинхронная корутина, которая задает значение через фиксированное время и шанс на повторное ожидание
    Task setValueAsync(int n) {
        // Генератор случайных чисел для шанса на повторное ожидание
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> distrib(0, 1);  // Шанс 50%

        std::cout << "Задержка для объекта: 1 секунда\n";

        // Ожидаем 1 секунду асинхронно
        co_await sleep_for(std::chrono::seconds(1));

        // Если выпадает 50%, ждем еще 1 секунду
        if (distrib(gen) == 1) {
            std::cout << "Повторное ожидание для объекта: 1 секунда\n";
            co_await sleep_for(std::chrono::seconds(1));
        }

        // Устанавливаем новое значение value
        value = n;
        std::cout << "Значение объекта установлено в: " << value << std::endl;
    }
};

// Асинхронный событийный цикл (event loop)
void event_loop(std::deque<Task>& tasks) {
    while (!tasks.empty()) {
        for (auto it = tasks.begin(); it != tasks.end();) {
            it->resume();  // Резюмируем корутину
            if (it->done()) {
                it = tasks.erase(it);  // Убираем задачу, если она завершена
            } else {
                ++it;  // Переходим к следующей задаче
            }
        }
        // Ожидание перед следующей итерацией цикла для симуляции асинхронного выполнения
        std::this_thread::sleep_for(std::chrono::milliseconds(100));  // Небольшая пауза между проверками
    }
}

int main() {
    // Создаем несколько объектов класса A
    std::vector<A> objects(30);  // Создаем 30 объектов

    // Запускаем корутины для каждого объекта и сохраняем их для дальнейшего ожидания
    std::deque<Task> tasks;
    for (int i = 0; i < 30; ++i) {
        tasks.push_back(objects[i].setValueAsync(i));
    }

    // Запускаем событийный цикл, который обрабатывает все корутины
    event_loop(tasks);

    // Выводим сообщение о завершении всех задач
    std::cout << "Все задачи завершены." << std::endl;

    return 0;
}

