#include <iostream>
#include <vector>
#include <mutex>
#include <thread>
#include <chrono>
#include <format> // C++20

// Константы
const int DATA_SIZE = 100;       // Количество элементов для обработки
const int NUM_THREADS = 10;      // Количество рабочих потоков
const auto WORK_TIME = std::chrono::milliseconds(10); // Имитация тяжелой работы

class Processor {
    std::vector<int> source_data;    // Отсюда берем
    std::vector<int> processed_data; // Сюда кладем
    std::mutex mtx;

public:
    // Заполняем данными перед тестом
    void reset_data() {
        source_data.clear();
        processed_data.clear();
        for (int i = 0; i < DATA_SIZE; ++i) {
            source_data.push_back(i);
        }
    }

    // ==========================================
    // ВАРИАНТ 1: Медленный (Блокирует надолго)
    // ==========================================
    void process_slow() {
        while (true) {
            // [!] lock_guard захватывает мьютекс до конца скоупа (цикла while)
            // Но проблема в том, что lock создается на каждой итерации заново? 
            // Нет, проблема в том, что он держит мьютекс ВО ВРЕМЯ sleep_for.
            
            std::lock_guard lock(mtx);

            if (source_data.empty()) {
                break;
            }

            // 1. Взяли данные (быстро)
            int item = source_data.back();
            source_data.pop_back();

            // 2. Обработка (ДОЛГО!)
            // Мьютекс всё ещё захвачен. Другие потоки стоят и ждут.
            std::this_thread::sleep_for(WORK_TIME); 
            int result = item * 2;

            // 3. Запись результата (быстро)
            processed_data.push_back(result);
            
        } // Тут вызывается unlock
    }

    // ==========================================
    // ВАРИАНТ 2: Быстрый (Manual Unlock)
    // ==========================================
    void process_fast() {
        while (true) {
            // ЗАДАНИЕ:
            // Реализуйте этот метод, используя std::unique_lock.
            
            // Шаг 1: Захватите мьютекс
            std::unique_lock lock(mtx);
            
            // Шаг 2: Проверьте, есть ли данные. Если нет — выход.
            if (source_data.empty()) {
                break;
            }

            // Шаг 3: Скопируйте данные в локальную переменную и удалите из source_data
            int item = source_data.back();
            source_data.pop_back();

            // Шаг 4: [ГЛАВНОЕ] Разблокируйте мьютекс вручную!
            // --- ВАШ КОД: lock.unlock(); ---
            lock.unlock();

            // Шаг 5: Тяжелая работа (теперь параллельно!)
            std::this_thread::sleep_for(WORK_TIME);
            int result = item * 2;

            // Шаг 6: Снова заблокируйте мьютекс для записи результата
            // --- ВАШ КОД: lock.lock(); ---
            lock.lock();

            // Шаг 7: Запишите результат
            processed_data.push_back(result);
            
            // Шаг 8: В конце цикла lock сам сделает unlock, если нужно, 
            // но мы уже сделали lock() на шаге 6, так что всё ок.
        }
    }

    size_t get_processed_count() const {
        return processed_data.size();
    }
};

// Функция бенчмарка
template <typename Func>
void run_benchmark(Processor& p, Func func, const std::string& name) {
    p.reset_data();
    
    auto start = std::chrono::high_resolution_clock::now();
    
    std::vector<std::thread> threads;
    for (int i = 0; i < NUM_THREADS; ++i) {
        threads.emplace_back(func, &p); // Запускаем потоки
    }

    for (auto& t : threads) t.join();

    auto end = std::chrono::high_resolution_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    std::cout << std::format("[{}] Time: {} ms | Processed: {} items\n", 
                             name, ms, p.get_processed_count());
}

int main() {
    Processor processor;

    std::cout << "Starting optimization benchmark...\n";
    std::cout << "Items to process: " << DATA_SIZE << "\n";
    std::cout << "Work per item: " << WORK_TIME.count() << "ms\n";
    
    // Ожидаемое время для SLOW: DATA_SIZE * WORK_TIME (так как потоки работают по очереди)
    // Ожидаемое время для FAST: (DATA_SIZE * WORK_TIME) / NUM_THREADS (параллельно)

    run_benchmark(processor, &Processor::process_slow, "Slow (lock_guard) ");
    run_benchmark(processor, &Processor::process_fast, "Fast (unique_lock)");

    return 0;
}