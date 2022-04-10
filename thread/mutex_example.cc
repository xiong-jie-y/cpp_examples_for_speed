// Mutex example.
// 
// The output should be 200000, but if you remove lock_guard, 
// the output will be different.
#include <iostream>
#include <mutex>
#include <thread>

// Count enough to have data racing when there's no
void count_enough(int* count, std::mutex* mutex) {
    constexpr int REPEAT_COUNT = 100000;

    for (int i = 0; i < REPEAT_COUNT; ++i) {
        std::lock_guard<std::mutex> guard(*mutex);
        *count += 1;
    }
}

int main() {
    int count = 0;
    std::mutex mutex;

    // TODO: why it's impossible to pass arguments by reference.
    std::thread t1(count_enough, &count, &mutex);
    std::thread t2(count_enough, &count, &mutex);

    t1.join();
    t2.join();

    std::cout << count << std::endl;
}