#pragma once

#include <glm/glm.hpp>
#include <memory>
#include <limits>
#include "Path.h"

struct Hitbox {
    glm::vec3 center;
    float     radius;

    bool intersects(const Hitbox& o) const {
        float r = radius + o.radius;
        glm::vec3 d = center - o.center;
        return glm::dot(d, d) <= r*r;
    }
};

enum class EnemyAttribute {
    FAST,
    FLYING,
    RESISTANT
};

struct IMovement {
    virtual ~IMovement() = default;
    virtual void move(class Enemy& e, float dt) = 0;
};

class Enemy {
public:
    Hitbox                     hitbox;
    float                      health;
    float                      baseSpeed;
    EnemyAttribute             attribute;
    bool                       alive = true;
    std::unique_ptr<IMovement> movement;
    float                      distanceTraveled = 0.0f;
    float                      reward;
    
    Enemy(const glm::vec3& pos, float radius, float hp, float speed, EnemyAttribute attr, int rewardValue, std::unique_ptr<IMovement> mv)
      : hitbox{pos, radius}, 
        health(hp), 
        baseSpeed(speed), 
        attribute(attr), 
        reward(rewardValue), 
        movement(std::move(mv)) {}

    float speed() const {
        if (attribute == EnemyAttribute::FAST) return baseSpeed * 1.5f;
        return baseSpeed;
    }

    void update(float dt) {
        if (!alive) return;
        if (movement) movement->move(*this, dt);
    }

    void applyDamage(float dmg) {
        if (attribute == EnemyAttribute::RESISTANT) dmg *= 0.5f;
        health -= dmg;
        if (health <= 0.0f) alive = false;
    }
};

struct BezierMovement : IMovement {
    std::shared_ptr<Path> caminho;

    BezierMovement(std::shared_ptr<Path> c) : caminho(std::move(c)) {}

    void move(Enemy& e, float dt) override {
        if (!caminho) return;
        e.distanceTraveled += e.speed() * dt;
        if (e.distanceTraveled >= caminho->getTotalLength()) {
            e.hitbox.center = caminho->getEndPoint();
            e.alive = false;                   
            e.reward = 0; 
            printf("Um inimigo chegou a base!\n"); 
            return;
        }
        float t = caminho->getTForDistance(e.distanceTraveled);
        e.hitbox.center = caminho->getPoint(t);
    }
};