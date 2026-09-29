#pragma once

#include <functional>
#include <string>
#include <utility>
#include <sstream>

class ILoggable
{
public:
  using LogFn = std::function<void(const std::string &)>;

  virtual ~ILoggable() = default;

  void setLogger(LogFn fn)
  {
    logFn = fn ? std::move(fn) : [](const std::string &) {};
  }

protected:
  class LogStream
  {
  public:
    explicit LogStream(const LogFn &logFn) : logFn(logFn) {}

    ~LogStream()
    {
      logFn(stream.str());
    }

    template <typename T>
    LogStream &operator<<(const T &value)
    {
      stream << value;
      return *this;
    }

  private:
    const LogFn &logFn;
    std::ostringstream stream;
  };

  LogStream log() const
  {
    return LogStream(logFn);
  }

private:
  LogFn logFn = [](const std::string &) {};
};