// A Conditional variable example.
// 
// This is the example where two consumers wait for the data produced by producer.
// 
#include <chrono>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <thread>

void consumer(bool* produced, std::mutex* mutex, std::condition_variable* cv) {
    std::unique_lock<std::mutex> lock(*mutex);
    cv->wait(lock, [&]{ return *produced; });

    std::cout << "produced: " << *produced << std::endl;
}

void producer(bool* produced, std::mutex* mutex, std::condition_variable* cv) {
    // Emulate long process.
    std::this_thread::sleep_for(std::chrono::seconds(1));

    // TODO: Validate if the lock here is really necessary for this program.
    {
        std::unique_lock<std::mutex> lock(*mutex);
        *produced = true;
    }

    cv->notify_all();
}

int main() {
    bool produced = false;
    std::mutex mutex;
    std::condition_variable cv;

    std::thread pth(producer, &produced, &mutex, &cv);
    std::thread cth0(consumer, &produced, &mutex, &cv);
    std::thread cth1(consumer, &produced, &mutex, &cv);

    pth.join();
    cth0.join();
    cth1.join();
}