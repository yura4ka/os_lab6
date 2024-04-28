#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>
#include <functional>

const int N = 1e6;
std::mutex mut;

void incWoLock(int &x) {
    for (int i = 0; i < N; i++) {
        x++;
    }
}

void incLock(int &x) {
    for (int i = 0; i < N; i++) {
        mut.lock();
        x++;
        mut.unlock();
    }
}

void calculate(const std::function<void(int &)> &f) {
    int x = 0;
    auto startTime = std::chrono::high_resolution_clock::now();
    std::thread thread1(f, std::ref(x));
    std::thread thread2(f, std::ref(x));
    thread1.join();
    thread2.join();
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
    std::cout << "x = " << x << "; duration = " << duration.count() << "ms\n";
}

void fastest() {
    int x1 = 0, x2 = 0;
    auto startTime = std::chrono::high_resolution_clock::now();
    std::thread thread1(incWoLock, std::ref(x1));
    std::thread thread2(incWoLock, std::ref(x2));
    thread1.join();
    thread2.join();
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
    std::cout << "x = " << x1 + x2 << "; duration = " << duration.count() << "ms\n";
}

int main() {
    std::cout << "2a) without lock:\n";
    calculate(incWoLock);

    std::cout << "2b) with lock:\n";
    calculate(incLock);

    std::cout << "2.2 fastest:\n";
    fastest();
}
