#pragma once
#include "drone_link.h"

class IUartListener {
  public:
    virtual void updateTelemetry(const dlink::Telemetry &telemetry) = 0;
    virtual void updateTargetPosition(const dlink::TargetPos &targetPosition) = 0;
    virtual void updateAmmoConfig(const dlink::AmmoCfg &ammoConfig) = 0;
    virtual void updateResult(const dlink::Result &result) = 0;
    virtual void updateDroneConfig(const dlink::DroneCfg &droneConfig) = 0;
    virtual void updateControl(const dlink::Control &control) = 0;
    virtual ~IUartListener() = default;
};