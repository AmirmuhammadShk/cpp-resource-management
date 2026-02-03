#include <cstdio>
#include <cstring>
#include <mutex>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

// RAII wrapper for FILE*
class FileHandle {
public:
    explicit FileHandle(const char* path, const char* mode) : fp_(std::fopen(path, mode)) {
        if (!fp_) throw std::runtime_error("Failed to open file");
    }

    // non-copyable
    FileHandle(const FileHandle&) = delete;
    FileHandle& operator=(const FileHandle&) = delete;

    // movable
    FileHandle(FileHandle&& other) noexcept : fp_(other.fp_) { other.fp_ = nullptr; }
    FileHandle& operator=(FileHandle&& other) noexcept {
        if (this != &other) {
            close();
            fp_ = other.fp_;
            other.fp_ = nullptr;
        }
        return *this;
    }

    ~FileHandle() { close(); }

    void write_line(const std::string& line) {
        if (!fp_) throw std::runtime_error("File is closed");
        std::fputs(line.c_str(), fp_);
        std::fputc('\n', fp_);
        std::fflush(fp_);
    }

private:
    void close() noexcept {
        if (fp_) {
            std::fclose(fp_);
            fp_ = nullptr;
        }
    }
    std::FILE* fp_{nullptr};
};

// RAII lock guard (you can also use std::lock_guard, but this is explicit for the demo)
class ScopedLock {
public:
    explicit ScopedLock(std::mutex& m) : m_(m) { m_.lock(); }
    ScopedLock(const ScopedLock&) = delete;
    ScopedLock& operator=(const ScopedLock&) = delete;
    ~ScopedLock() { m_.unlock(); }
private:
    std::mutex& m_;
};

class Logger {
public:
    explicit Logger(const char* path) : file_(path, "a") {}

    void log(std::string msg) {
        ScopedLock lock(mu_);
        file_.write_line(msg);
    }

    void log_then_throw(std::string msg) {
        ScopedLock lock(mu_);
        file_.write_line(msg);
        throw std::runtime_error("Intentional exception after writing");
    }

private:
    FileHandle file_;
    std::mutex mu_;
};

int main() {
    try {
        Logger logger("app.log");

        // Start a few threads to demonstrate scoped locking + safe file close.
        std::vector<std::thread> threads;
        for (int i = 0; i < 3; ++i) {
            threads.emplace_back([&logger, i] {
                for (int j = 0; j < 5; ++j) {
                    logger.log("thread " + std::to_string(i) + " message " + std::to_string(j));
                }
            });
        }
        for (auto& t : threads) t.join();

        // Prove exception safety: file is still closed on program exit, no leaks.
        logger.log_then_throw("about to throw (file will still be closed properly)");
    } catch (const std::exception& e) {
        std::printf("Caught exception: %s\n", e.what());
    }

    std::puts("Done. Check app.log");
    return 0;
}
