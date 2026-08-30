#pragma once

#include <string>

class IParser
{
public:
  virtual auto parse(const std::string &data) -> void = 0;
  virtual ~IParser() = default;
};