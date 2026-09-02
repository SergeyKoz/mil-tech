#pragma once

#include <thread>
#include <vector>
#include <memory>
#include "common.hpp"

class IParser;

class UartReader
{
public:
  UartReader(const std::string &uartPort);

  void start();
  void wait();
  bool isThreadReady() const;
  void stop();

  auto addParser(const std::shared_ptr<IParser> &parser) -> void;

  virtual ~UartReader();

private:
  std::string uartPort;
  int uart = -1;

  std::thread receiveThread;
  std::atomic<bool> isRunning{false};

  std::vector<std::weak_ptr<IParser>> parsers;

  bool isInited{false};

  auto receiveLoop() -> void;
  auto handleTelemetry(const std::string &telemetry) -> void;

  void init();
};
