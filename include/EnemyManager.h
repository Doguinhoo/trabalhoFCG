#pragma once

#include <vector>
#include <memory>
#include "Enemy.h"

// Header das funções 
class EnemyManager {
public:
    EnemyManager() = default;
    ~EnemyManager() = default;

    void spawn(std::unique_ptr<Enemy> enemy);
    int updateAll(float dt, float& playerMoney); 
    void clearAll();
    std::vector<Enemy*> getEnemyPointers() const;

private:
    void cleanup(float& playerMoney);
    std::vector<std::unique_ptr<Enemy>> enemies;
};