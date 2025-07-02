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

    std::unique_ptr<Tower> buy(const std::string& name, float& playerMoney, const glm::vec3& pos) {
        auto it = blueprints.find(name);
        if (it == blueprints.end() || playerMoney < it->second.cost) return nullptr;
        
        const auto& bp = it->second;
        playerMoney -= bp.cost;
        
        auto targeting = bp.targetingFactory ? bp.targetingFactory() : nullptr;
        auto shooting = bp.shootingFactory ? bp.shootingFactory() : nullptr;
        auto passive = bp.passiveFactory ? bp.passiveFactory() : nullptr;

        return std::make_unique<Tower>(bp.name, pos, bp.range, bp.cooldown, std::move(targeting), std::move(shooting), std::move(passive));
    }

    std::unique_ptr<Tower> upgrade(Tower& oldTower, float& playerMoney) {
        auto it_current = blueprints.find(oldTower.blueprintName);
        if (it_current == blueprints.end()) return nullptr;

        const auto& current_bp = it_current->second;
        if (current_bp.nextUpgradeName.empty() || playerMoney < current_bp.upgradeCost) return nullptr;

        auto it_next = blueprints.find(current_bp.nextUpgradeName);
        if (it_next == blueprints.end()) return nullptr;
        
        playerMoney -= current_bp.upgradeCost;
        const auto& next_bp = it_next->second;

        auto targeting = next_bp.targetingFactory ? next_bp.targetingFactory() : nullptr;
        auto shooting = next_bp.shootingFactory ? next_bp.shootingFactory() : nullptr;
        auto passive = next_bp.passiveFactory ? next_bp.passiveFactory() : nullptr;

        return std::make_unique<Tower>(next_bp.name, oldTower.pos, next_bp.range, next_bp.cooldown, std::move(targeting), std::move(shooting), std::move(passive));
    }

    std::vector<std::string> availableTowers() const {
        std::vector<std::string> out;
        out.reserve(blueprints.size());
        for (const auto& kv : blueprints) {
            if (kv.second.cost > 0) {
                 out.push_back(kv.first);
            }
        }
        return out;
    }

private:
    std::unordered_map<std::string, TowerBlueprint> blueprints;
};