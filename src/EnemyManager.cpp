#include "EnemyManager.h"
#include <algorithm>  // std::remove_if

void EnemyManager::spawn(std::unique_ptr<Enemy> enemy) {
    enemies.emplace_back(std::move(enemy));
}

void EnemyManager::updateAll(float dt) {
    for (auto& e : enemies) {
        e->update(dt);
    }
    cleanup();
}

void EnemyManager::renderAll() const {
    for (const auto& e : enemies) {
        // ex: DrawSphere(e->hitbox.center, e->hitbox.radius);
    }
}

std::vector<Enemy*> EnemyManager::getEnemyPointers() const {
    std::vector<Enemy*> out;
    out.reserve(enemies.size());
    for (const auto& e : enemies) {
        out.push_back(e.get());
    }
    return out;
}

void EnemyManager::cleanup() {
    enemies.erase(
        std::remove_if(
            enemies.begin(), enemies.end(),
            [](const std::unique_ptr<Enemy>& e) {
                return !e->alive;
            }
        ),
        enemies.end()
    );
}
