#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>
#include <mutex>
#include <atomic>
#include <functional>
#include <ctime>
#include <iomanip>

class Logger {
public:
    Logger(const std::string& filename) : log_file(filename, std::ios::app) {
        if (!log_file.is_open()) {
            std::cerr << "Failed to open log file!" << std::endl;
        }
    }
    ~Logger() { log_file.close(); }

    void log(const std::string& message) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto now = std::chrono::system_clock::now();
        auto now_time_t = std::chrono::system_clock::to_time_t(now);
        auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
        log_file << std::put_time(std::localtime(&now_time_t), "%Y-%m-%d %H:%M:%S") << '.'
                 << std::setfill('0') << std::setw(3) << now_ms.count() << " [INFO] " << message << std::endl;
    }

private:
    std::ofstream log_file;
    std::mutex mutex_;
};