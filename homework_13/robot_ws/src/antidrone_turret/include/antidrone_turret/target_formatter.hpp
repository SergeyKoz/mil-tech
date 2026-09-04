#pragma once

#include <string>

#include "antidrone_turret/msg/target.hpp"

namespace antidrone_turret {

inline std::string target_label(const msg::Target& target)
{
    if (!target.visible) {
        return "hidden";
    }

    if (target.confidence < 0.8F) {
        return "low_confidence";
    }

    return "tracked";
}

}  // namespace antidrone_turret
