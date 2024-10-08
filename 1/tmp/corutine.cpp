#include <iostream>
#include <coroutine>
#include <vector>
#include <chrono>

struct Coroutine {
    struct promise_type {
        Coroutine get_return_object() {
            return Coroutine{std::coroutine_handle<promise_type>::from_promise(*this)};
        }
        std::suspend_never initial_suspend() { return {}; }
        std::suspend_always final_suspend() noexcept { return {}; }
        void unhandled_exception() { std::exit(1); }
        void return_void() {}
        std::suspend_always yield_value(int value) {
            this->value = value;
            return {};
        }
        int value;
    };

    std::coroutine_handle<promise_type> handle;

    Coroutine(std::coroutine_handle<promise_type> h) : handle(h) {}
    ~Coroutine() { handle.destroy(); }

    bool resume() {
        if (handle) {
            handle.resume();
            return !handle.done();
        }
        return false;
    }

    int getValue() {
        return handle.promise().value;
    }
};

Coroutine myCoroutine(int id) {
    for (int i = 0; i < 3; ++i) {
        co_yield i + id; // Возвращает значение с добавлением id
        // Здесь можно добавить логику, которая будет выполняться между yield
    }
}

int main() {
    std::vector<Coroutine> coroutines;
	std::cout << "tut\n";
    // Создаем несколько корутин
    for (int i = 0; i < 5; ++i) {
        coroutines.emplace_back(myCoroutine(i));
    }

    bool all_done = false;
	std::cout << "tut\n";
    // Цикл выполнения корутин
    while (!all_done) {
        all_done = true; // Предполагаем, что все корутины завершены

        for (auto& coro : coroutines) {
            if (coro.resume()) {
                std::cout << "Coroutine yielded: " << coro.getValue() << "\n";
                all_done = false; // Если хотя бы одна корутина не завершена
            }
        }

        // Здесь можно добавить дополнительные действия, которые будут выполняться, пока корутины работают
    }

    std::cout << "All coroutines have finished." << std::endl;

    return 0;
}

