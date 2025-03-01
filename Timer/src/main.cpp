#include "../include/Log.hpp"
#include "../include/Timer.hpp"
#include <random>

Logger logger("log.txt");

// 回调函数：记录执行时间和线程 ID
int callback() {
    std::thread::id tid = std::this_thread::get_id();
    // logger.log("Callback executed by thread " + std::to_string(std::hash<std::thread::id>{}(tid)));
    return 20; // 每 ms 执行一次
}

// 工作线程：随机调用 start 或 stop
void worker(Timer& timer, int id, std::atomic<bool>& running) {
    std::default_random_engine gen(id + std::random_device{}());
    std::uniform_int_distribution<int> action_dist(0, 1); // 0: start, 1: stop
    std::uniform_int_distribution<int> delay_dist(0, 100); // 延迟 0-100ms

    int ops = 0;
    while (running) {
        if (action_dist(gen) == 0) {
            timer.start(callback, delay_dist(gen));
            // logger.log("Thread " + std::to_string(id) + ": Start with delay " + std::to_string(delay_dist(gen)) + " ms");
        } else {
            timer.stop();
            // logger.log("Thread " + std::to_string(id) + ": Stop");
        }
        ops++;
        std::this_thread::sleep_for(std::chrono::microseconds(1)); // 微小休眠，增加操作频率
    }
    logger.log("Thread " + std::to_string(id) + " executed " + std::to_string(ops) + " operations");
}

int main() {
    Timer timer;
    std::atomic<bool> running(true);
    const int thread_count = 50;

    std::vector<std::thread> threads;
    for (int i = 0; i < thread_count; ++i) {
        threads.emplace_back(worker, std::ref(timer), i, std::ref(running));
    }

    std::this_thread::sleep_for(std::chrono::seconds(20));
    running = false;

    for (auto& t : threads) {
        t.join();
    }

    timer.stop();
    std::this_thread::sleep_for(std::chrono::milliseconds(5000)); // 等待回调完成

    logger.log("Test completed");
    std::cout << "测试完成，日志已写入 log.txt" << std::endl;

    
    return 0;
}