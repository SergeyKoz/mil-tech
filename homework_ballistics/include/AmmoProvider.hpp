#pragma once

#include <string>

struct AmmoParams {
    char name[32];
    float mass;  // маса (кг)
    float drag;  // коефіцієнт опору
    float lift;  // коефіцієнт підйому
};

struct AmmoList {
    int count;
    AmmoParams* ammo;
};

class AmmoProvider {
  public:
    AmmoProvider(const char* filePath);
    AmmoList getAmmoList();

  private:
    std::string configFilePath;
};