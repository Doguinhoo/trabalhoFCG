#pragma once

#include <vector>
#include <memory>
#include "Enemy.h"

class EnemyManager {
public:
    EnemyManager() = default;
    ~EnemyManager() = default;

    /// Spawn de um novo inimigo
    void spawn(std::unique_ptr<Enemy> enemy);

    /// Atualiza todos os inimigos e faz a limpeza dos mortos
    void updateAll(float dt);

    /// Renderiza todos os inimigos (você integra aqui com OpenGL)
    void renderAll() const;

    /// Retorna ponteiros para todas as entidades vivas
    std::vector<Enemy*> getEnemyPointers() const;

private:
    /// Remove inimigos com alive==false
    void cleanup();

    std::vector<std::unique_ptr<Enemy>> enemies;
};
