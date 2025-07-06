#include "EnemyManager.h"
#include <algorithm>

void EnemyManager::spawn(std::unique_ptr<Enemy> enemy) {
    enemies.emplace_back(std::move(enemy));
}

int EnemyManager::updateAll(float dt, float& playerMoney) {
    // Atualiza todos os inimigos primeiro
    for (auto& e : enemies) {
        e->update(dt);
    }

    int enemies_that_finished = 0;

    enemies.erase(
        std::remove_if(
            enemies.begin(), enemies.end(),
            [&playerMoney, &enemies_that_finished](const std::unique_ptr<Enemy>& e) {
                if (!e->alive) { 
                    if (e->killedByPlayer) {
                        // Foi morto por uma torre, dá dinheiro
                        playerMoney += e->reward;
                        printf("+%d de dinheiro! Inimigo derrotado.\n", e->reward);
                    } else {
                        // Chegou ao fim do caminho, não dá dinheiro e incrementa o contador
                        enemies_that_finished++;
                    }
                    return true; 
                }
                return false;
            }
        ),
        enemies.end()
    );

    return enemies_that_finished;
}

void EnemyManager::cleanup(float& playerMoney) {
    enemies.erase(
        std::remove_if(
            enemies.begin(), enemies.end(),
            [&playerMoney](const std::unique_ptr<Enemy>& e) {
                if (!e->alive) {
                    // Se o inimigo está morto, adiciona a recompensa
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

void EnemyManager::clearAll() {
    enemies.clear();
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
