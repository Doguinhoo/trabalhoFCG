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

        return std::unique_ptr<Tower>(new Tower(
            bp.name, bp.modelName, pos, bp.range, bp.cooldown,
            std::move(targeting), std::move(shooting), std::move(passive)
        ));
    }

    std::unique_ptr<Tower> upgrade(Tower& oldTower, float& playerMoney) {
        // Encontra o blueprint da torre ATUAL. Se não achar, falha.
        auto it_current = blueprints.find(oldTower.blueprintName);
        if (it_current == blueprints.end()) {
            return nullptr;
        }
        const auto& current_bp = it_current->second;

        // Verifica se a torre atual PODE ser melhorada. Se não tiver nome de upgrade
        //  ou se o jogador não tiver dinheiro, falha.
        if (current_bp.nextUpgradeName.empty() || playerMoney < current_bp.upgradeCost) {
            return nullptr;
        }

        // Encontra o blueprint da PRÓXIMA torre. Se não achar, falha.
        auto it_next = blueprints.find(current_bp.nextUpgradeName);
        if (it_next == blueprints.end()) {
            return nullptr;
        }
        
        // Somente se todas as verificações acima passarem, deduz o dinheiro.
        playerMoney -= current_bp.upgradeCost;

        // Cria e retorna a nova torre.
        const auto& next_bp = it_next->second;
        auto targeting = next_bp.targetingFactory ? next_bp.targetingFactory() : nullptr;
        auto shooting = next_bp.shootingFactory ? next_bp.shootingFactory() : nullptr;
        auto passive = next_bp.passiveFactory ? next_bp.passiveFactory() : nullptr;

        return std::unique_ptr<Tower>(new Tower(
            next_bp.name, next_bp.modelName, oldTower.pos, next_bp.range, next_bp.cooldown,
            std::move(targeting), std::move(shooting), std::move(passive)
        ));
    }
    std::vector<std::string> availableTowers() const {
        std::vector<std::string> out;
        out.reserve(blueprints.size());
        for (const auto& bp : blueprints) {
            if (bp.second.cost > 0) {
                 out.push_back(bp.first);
            }
        }
        return out;
    }

private:
    std::unordered_map<std::string, TowerBlueprint> blueprints;
};