#pragma once

#include <chrono>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <string>
#include <string_view>
#include <optional>
#include <mutex>
#include <thread>

using namespace std::literals;
#ifdef __linux__
    static const std::string log_dir = "/var/log"s;
#else 
    static const std::string log_dir = "./logs/"s;
#endif

//static const std::string log_dir = "C:\\Projects\\yandex_practicum\\cppbackend\\sprint2\\problems\\logger\\solution\\build\\var\\log";


#define LOG(...) Logger::GetInstance().Log(__VA_ARGS__)

class Logger {
    auto GetTime() const {
        if (manual_ts_) {
            return *manual_ts_;
        }

        return std::chrono::system_clock::now();
    }

    auto GetTimeStamp() const {
        const auto now = GetTime();
        const auto t_c = std::chrono::system_clock::to_time_t(now);
        return std::put_time(std::localtime(&t_c), "%F %T");
    }

    // Для имени файла возьмите дату с форматом "%Y_%m_%d"
    std::string GetFileTimeStamp() const {
        std::ostringstream time;
        const auto t_c = std::chrono::system_clock::to_time_t(GetTime());
        time << std::put_time(std::localtime(&t_c), "%Y_%m_%d");
        return time.str();
    }

    Logger() = default;
    Logger(const Logger&) = delete;

public:
    static Logger& GetInstance() {
        static Logger obj;
        return obj;
    }


    template <typename T0, typename... Ts>
    void LogImpl(std::ofstream& file, const T0& v0, const Ts&... vs) {
        using namespace std::literals;
      
        file << v0;
        // Выводим через запятую остальные параметры, если они остались
        if constexpr (sizeof...(vs) != 0) {
            LogImpl(file, vs...);  // Рекурсивно выводим остальные параметры
        }
    }

    // Выведите в поток все аргументы.
    template<class... Ts>
    void Log(const Ts&... args) {
        std::lock_guard lock(m_);

        std::string path = log_dir + "/sample_log_"s + GetFileTimeStamp() + ".log"s;

        std::ofstream log_file_(path, std::ios::app);
        

        if (!log_file_.is_open()) {
            return;
        }

        log_file_ << GetTimeStamp() << ": "s;
        if constexpr (sizeof...(args) != 0) {
            LogImpl(log_file_, args...);
        }
        log_file_ << std::endl;
    }

    // Установите manual_ts_. Учтите, что эта операция может выполняться
    // параллельно с выводом в поток, вам нужно предусмотреть 
    // синхронизацию.
    void SetTimestamp(std::chrono::system_clock::time_point ts) {
        std::lock_guard lock(m_);
        manual_ts_ = std::move(ts);
    }

private:
    std::optional<std::chrono::system_clock::time_point> manual_ts_;
    std::mutex m_;
    
};
