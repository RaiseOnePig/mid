#include <atomic>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <thread>
#include <cstdio> // 用于 fprintf

class Timer
{
public:
    Timer() : state_(STOPPED), running_(true), thread_(&Timer::run, this) {}

    // 析构函数，确保线程安全退出
    ~Timer()
    {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            running_ = false;
            state_ = STOPPED;
            cv_.notify_one();
        }
        if (thread_.joinable()) {
            thread_.join(); // 无需 try-catch，joinable 已检查
            fprintf(stdout, "[DEBUG] Timer destructed and thread joined.\n");
        }
    }

    // 启动定时器，重复调用会重置任务
    void start(std::function<int()> callback, int delay_ms = 0)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!running_) {
            return;
        }
        if (state_ == RUNNING) {
            fprintf(stdout, "[DEBUG] Timer already running, resetting with new parameters.\n");
        }
        callback_ = callback;
        delay_ms_ = delay_ms;
        state_ = RUNNING;
        cv_.notify_one();
        fprintf(stdout, "[DEBUG] Timer started/reset with delay %d ms.\n", delay_ms);
    }

    // 停止定时器
    void stop()
    {
        std::lock_guard<std::mutex> lock(mutex_);
        state_ = STOPPED;       // 切换状态为停止
        cv_.notify_one();       // 通知线程
    }

private:
    enum State { STOPPED, RUNNING };  // 定时器状态
    std::atomic<State> state_;        // 原子状态变量
    std::function<int()> callback_;   // 回调函数
    int delay_ms_;                    // 延迟时间（毫秒）
    std::mutex mutex_;                // 互斥锁
    std::condition_variable cv_;      // 条件变量
    std::thread thread_;              // 定时器线程
    std::atomic<bool> running_;       // 线程运行标志

    // 定时器线程的运行逻辑
    void run()
    {
        while (running_) {
            std::unique_lock<std::mutex> lock(mutex_);
            // 等待状态为 RUNNING 或收到退出信号
            cv_.wait(lock, [this] { return state_ == RUNNING || !running_; });

            // 如果收到退出信号，结束线程
            if (!running_) {
                break;
            }

            // 如果状态为 STOPPED，继续等待
            if (state_ == STOPPED) {
                continue;
            }

            int delay = delay_ms_;      // 保存当前延迟时间
            auto cb = callback_;        // 保存当前回调函数
            lock.unlock();              // 解锁，避免阻塞其他操作

            // 睡眠指定的延迟时间
            std::this_thread::sleep_for(std::chrono::milliseconds(delay));
            fprintf(stdout, "[DEBUG] Timer woke up after %d ms.\n", delay);

            lock.lock();                // 重新加锁
            if (state_ == STOPPED || !running_) {
                fprintf(stdout, "[DEBUG] Timer was stopped during sleep or destruction.\n");
                continue;
            }

            try {
                int interval = cb();    // 执行回调
                if (interval <= 0) {
                    state_ = STOPPED;   // 回调返回 0 或负数，停止定时器
                }
                else {
                    delay_ms_ = interval; // 更新延迟时间，继续运行
                    state_ = RUNNING;
                }
            }
            catch (const std::exception &e) {
                fprintf(stderr, "[ERROR] Exception in callback: %s\n", e.what());
                state_ = STOPPED;       // 停止定时器
            }
            catch (...) {
                fprintf(stderr, "[ERROR] Unknown exception in callback.\n");
                state_ = STOPPED;       // 停止定时器
            }
        }
        fprintf(stdout, "[DEBUG] Timer thread exiting.\n");
    }
};
