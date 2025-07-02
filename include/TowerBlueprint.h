// include/TowerBlueprint.h
#pragma once

#include <string>
#include <functional>
#include <memory>
#include <glm/glm.hpp>
#include "Tower.h"

struct TowerBlueprint {
    std::string name;
    float       cost;
    float       range;
    float       cooldown;
    std::function<std::unique_ptr<ITargeting>()> targetingFactory;
    std::function<std::unique_ptr<IShooting>()>  shootingFactory;
};
