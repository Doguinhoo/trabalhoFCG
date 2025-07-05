#include "EnemyManager.h"
#include <algorithm>

void EnemyManager::spawn(std::unique_ptr<Enemy> enemy) {
    enemies.emplace_back(std::move(enemy));
}

void EnemyManager::updateAll(float dt, float& playerMoney) {
    for (auto& e : enemies) {
        e->update(dt);
    }
    cleanup(playerMoney);
}

void EnemyManager::cleanup(float& playerMoney) {
    enemies.erase(
        std::remove_if(
            enemies.begin(), enemies.end(),
            // A lambda agora captura playerMoney por referência
            [&playerMoney](const std::unique_ptr<Enemy>& e) {
                if (!e->alive) {
                    // Se o inimigo está morto, adiciona a recompensa!
                    playerMoney += e->reward;
                    printf("+%d de dinheiro! Inimigo derrotado.\n", e->reward);
                    return true; // Retorna true para marcar para remoção
                }
                return false;
            }
        ),
        enemies.end()
    );
}

std::vector<Enemy*> EnemyManager::getEnemyPointers() const {
    std::vector<Enemy*> out;
    out.reserve(enemies.size());
    for (const auto& e : enemies) {
        if (e->alive) {
            out.push_back(e.get());
        }
    }
    return out;
}
