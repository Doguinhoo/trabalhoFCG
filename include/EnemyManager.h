#pragma once

#include <vector>
#include <memory>
#include "Enemy.h"

class EnemyManager {
public:
    EnemyManager() = default;
    ~EnemyManager() = default;

    void spawn(std::unique_ptr<Enemy> enemy);
    void updateAll(float dt, float& playerMoney);
    void renderAll() const;
    std::vector<Enemy*> getEnemyPointers() const;

private:
    void cleanup(float& playerMoney);
    std::vector<std::unique_ptr<Enemy>> enemies;
};