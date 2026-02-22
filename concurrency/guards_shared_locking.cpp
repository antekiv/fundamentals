#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <shared_mutex>
#include <chrono>
#include <format>
#include <random>

const int NUM_READERS = 10;
const int WRITE_ITERATIONS = 5000;
const int READ_ITERATIONS = 20000;

class SimpleConfig {
    std::map<std::string, int> settings;
    mutable std::mutex mtx;

public:
    int get(const std::string& key) const {
        std::lock_guard lock(mtx);
        
        std::this_thread::sleep_for(std::chrono::nanoseconds(50));
        
        auto it = settings.find(key);
        return (it != settings.end()) ? it->second : -1;
    }

    void update(const std::string& key, int value) {
        std::lock_guard lock(mtx);
        
        std::this_thread::sleep_for(std::chrono::nanoseconds(50));
        
        settings[key] = value;
    }

    bool check_integrity(int expected_count) const {
        std::lock_guard lock(mtx);

        if (settings.size() != expected_count) {
            std::cerr << std::format("Error: Size mismatch! Expected {}, got {}\n", 
                                     expected_count, settings.size());
            return false;
        }

        for (int i = 0; i < expected_count; ++i) {
            std::string key = "key_" + std::to_string(i);
            
            auto it = settings.find(key);
            if (it == settings.end()) {
                 std::cerr << std::format("Error: Key {} missing!\n", key);
                 return false;
            }
            if (it->second != i) {
                 std::cerr << std::format("Error: Value mismatch for {}! Expected {}, got {}\n", 
                                          key, i, it->second);
                 return false;
            }
        }
        return true;
    }
};

class SharedConfig {
    std::map<std::string, int> settings;
    mutable std::shared_mutex mtx;

public:
    int get(const std::string& key) const {
        std::shared_lock lock(mtx); 
        
        std::this_thread::sleep_for(std::chrono::nanoseconds(50));
        
        auto it = settings.find(key);
        return (it != settings.end()) ? it->second : -1;
    }

    void update(const std::string& key, int value) {
        std::unique_lock lock(mtx);
        
        std::this_thread::sleep_for(std::chrono::nanoseconds(50));
        
        settings[key] = value;
    }

    bool check_integrity(int expected_count) const {
        std::shared_lock lock(mtx);

        if (settings.size() != expected_count) {
            std::cerr << std::format("Error: Size mismatch! Expected {}, got {}\n", 
                                     expected_count, settings.size());
            return false;
        }

        for (int i = 0; i < expected_count; ++i) {
            std::string key = "key_" + std::to_string(i);
            
            auto it = settings.find(key);
            if (it == settings.end()) {
                 std::cerr << std::format("Error: Key {} missing!\n", key);
                 return false;
            }
            if (it->second != i) {
                 std::cerr << std::format("Error: Value mismatch for {}! Expected {}, got {}\n", 
                                          key, i, it->second);
                 return false;
            }
        }
        return true;
    }
};

template <typename ConfigType>
void run_benchmark(const std::string& name) {
    ConfigType config;
    auto start_time = std::chrono::high_resolution_clock::now();

    std::vector<std::thread> threads;

    threads.emplace_back([&config]() {
        for (int i = 0; i < WRITE_ITERATIONS; i += 2) {
            config.update("key_" + std::to_string(i), i);
        }
    });

    for (int i = 0; i < NUM_READERS; ++i) {
        threads.emplace_back([&config]() {
            for (int j = 0; j < READ_ITERATIONS; ++j) {
                config.get("key_" + std::to_string(j % WRITE_ITERATIONS));
            }
        });
    }

    threads.emplace_back([&config]() {
        for (int i = 1; i < WRITE_ITERATIONS; i += 2) {
            config.update("key_" + std::to_string(i), i);
        }
    });

    for (auto& t : threads) t.join();

    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();

    bool ok = config.check_integrity(WRITE_ITERATIONS);

    std::cout << std::format("[{}] Time: {:4} ms | Integrity: {}\n", 
                             name, duration, (ok ? "OK" : "FAILED"));
}

int main() {
    std::cout << std::format("Benchmark: Map grows to {} items. {} Readers.\n", 
                             WRITE_ITERATIONS, NUM_READERS);
    std::cout << "---------------------------------------------------------\n";
    
    run_benchmark<SimpleConfig>("std::mutex       ");
    run_benchmark<SharedConfig>("std::shared_mutex");

    return 0;
}