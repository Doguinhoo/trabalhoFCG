// include/Enemy.h
#pragma once

#include <glm/glm.hpp>
#include <memory>
#include <limits>

// 1) hitbox: esfera de colisão
struct Hitbox {
    glm::vec3 center;
    float     radius;

    bool intersects(const Hitbox& o) const {
        float r = radius + o.radius;
        glm::vec3 d = center - o.center;
        return glm::dot(d, d) <= r*r;
    }
};

// 2) Atributos únicos de inimigos
enum class EnemyAttribute {
    FAST,       // mais rápido
    FLYING,     // ignora obstáculos
    RESISTANT   // recebe menos dano
};

// 3) Interface de movimento
struct IMovement {
    virtual ~IMovement() = default;
    virtual void move(class Enemy& e, float dt) = 0;
};

// 4) Classe Enemy
class Enemy {
public:
    Hitbox                     hitbox;
    float                      health;
    float                      baseSpeed;
    EnemyAttribute             attribute;
    bool                       alive = true;
    std::unique_ptr<IMovement> movement;
    float                      distanceTraveled = 0.0f; // <-- ADICIONADO

    Enemy(const glm::vec3& pos,
          float     radius,
          float     hp,
          float     speed,
          EnemyAttribute attr,
          std::unique_ptr<IMovement> mv)
      : hitbox{pos, radius},
        health(hp),
        baseSpeed(speed),
        attribute(attr),
        movement(std::move(mv))
    {}

    // Velocidade ajustada para FAST
    float speed() const {
        if (attribute == EnemyAttribute::FAST)
            return baseSpeed * 1.5f;
        return baseSpeed;
    }

    // Atualiza cada frame
    void update(float dt) {
        if (!alive) return;
        if (movement) movement->move(*this, dt);
    }

    // Aplica dano (ajustado para RESISTANT) e marca morto
    void applyDamage(float dmg) {
        if (attribute == EnemyAttribute::RESISTANT)
            dmg *= 0.5f;
        health -= dmg;
        if (health <= 0.0f) alive = false;
    }
};

// 5) Movimentos concretos
struct LinearMovement : IMovement {
    glm::vec3 goal;
    LinearMovement(const glm::vec3& g) : goal(g) {}
    void move(Enemy& e, float dt) override {
        if (glm::distance(e.hitbox.center, goal) < 0.1f) return;

        glm::vec3 dir = glm::normalize(goal - e.hitbox.center);
        float distanceThisFrame = e.speed() * dt;

        e.hitbox.center += dir * distanceThisFrame;
        e.distanceTraveled += distanceThisFrame; // <-- ATUALIZADO
    }
};

struct FlyingMovement : IMovement {
    glm::vec3 goal;
    FlyingMovement(const glm::vec3& g) : goal(g) {}
    void move(Enemy& e, float dt) override {
        if (glm::distance(e.hitbox.center, goal) < 0.1f) return;
        
        glm::vec3 dir = glm::normalize(goal - e.hitbox.center);
        float distanceThisFrame = e.speed() * dt;

        e.hitbox.center += dir * distanceThisFrame;
        e.distanceTraveled += distanceThisFrame; // <-- ATUALIZADO
    }
};