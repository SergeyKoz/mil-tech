#pragma once

#include <ctime>
#include <fstream>
#include <iomanip>
#include <mutex>
#include <sstream>
#include <string>
#include <iostream>

class Log {
public:
    explicit Log(const std::string& logFile)
        : file(logFile.c_str(), std::ios::app) {
    }
    ~Log(){
        if (file.is_open()) {
            file.close();
        }
    }

    void info(const std::string& message)
    {
        write("INFO", message);
    }

    void error(const std::string& message)
    {
        write("ERROR", message);
    }
private:
    std::ofstream file;
    std::mutex mutex;

    void write(const std::string& level, const std::string& message)
    {
        std::lock_guard<std::mutex> lock(mutex);

        if (!file.is_open()) {
            return;
        }

        std::cout << "[C2] [" << currentTime() << "] [" << level << "] " << message << "\n";
        file << "[C2] [" << currentTime() << "] [" << level << "] " << message << "\n";
        file.flush();
    }

    std::string currentTime() const
    {
        std::time_t now = std::time(NULL);
        std::tm* localTime = std::localtime(&now);

        std::ostringstream stream;
        stream << std::setfill('0')
               << std::setw(4) << localTime->tm_year + 1900 << "-"
               << std::setw(2) << localTime->tm_mon + 1 << "-"
               << std::setw(2) << localTime->tm_mday << " "
               << std::setw(2) << localTime->tm_hour << ":"
               << std::setw(2) << localTime->tm_min << ":"
               << std::setw(2) << localTime->tm_sec;

        return stream.str();
    }
};
