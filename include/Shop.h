// include/Shop.h
#pragma once

#include <memory>
#include <unordered_map>
#include <vector>
#include <string>
#include <glm/glm.hpp>
#include "TowerBlueprint.h"
#include "Tower.h"

class Shop {
public:
    void registerTower(const TowerBlueprint& bp) {
        blueprints[bp.name] = bp;
    }
    std::unique_ptr<Tower> buy(const std::string& name,
                               float& playerMoney,
                               const glm::vec3& pos)
    {
        auto it = blueprints.find(name);
        if (it == blueprints.end() || playerMoney < it->second.cost)
            return nullptr;
        const auto& bp = it->second;
        playerMoney -= bp.cost;
        return std::unique_ptr<Tower>(
            new Tower(pos, bp.range, bp.cooldown,
                      bp.targetingFactory(),
                      bp.shootingFactory())
        );
    }
    std::vector<std::string> availableTowers() const {
        std::vector<std::string> out;
        out.reserve(blueprints.size());
        for (auto& kv : blueprints) out.push_back(kv.first);
        return out;
    }
private:
    std::unordered_map<std::string, TowerBlueprint> blueprints;
};
