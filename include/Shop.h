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
        
        return std::make_unique<Tower>(
            bp.name, pos, bp.range, bp.cooldown,
            bp.targetingFactory(),
            bp.shootingFactory()
        );
    }

    std::unique_ptr<Tower> upgrade(Tower& oldTower, float& playerMoney) {
        auto it_current = blueprints.find(oldTower.blueprintName);
        if (it_current == blueprints.end()) return nullptr;

        const auto& current_bp = it_current->second;
        if (current_bp.nextUpgradeName.empty() || playerMoney < current_bp.upgradeCost) {
            return nullptr;
        }

        auto it_next = blueprints.find(current_bp.nextUpgradeName);
        if (it_next == blueprints.end()) return nullptr;

        playerMoney -= current_bp.upgradeCost;

        // Cria a nova torre usando a mesma posição da antiga
        const auto& next_bp = it_next->second;
        return std::make_unique<Tower>(
            next_bp.name, oldTower.pos, next_bp.range, next_bp.cooldown,
            next_bp.targetingFactory(),
            next_bp.shootingFactory()
        );
    }

    std::vector<std::string> availableTowers() const {
        std::vector<std::string> out;
        out.reserve(blueprints.size());
        for (const auto& kv : blueprints) out.push_back(kv.first);
        return out;
    }
private:
    std::unordered_map<std::string, TowerBlueprint> blueprints;
};