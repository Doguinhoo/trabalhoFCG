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
    float                      maxHealth;
    float                      baseSpeed;
    EnemyAttribute             attribute;
    bool                       alive = true;
    bool                       killedByPlayer = true;
    bool                       isSlowed = false; 
    float                      slowTimer = 0.0f;   
    std::unique_ptr<IMovement> movement;
    float                      distanceTraveled = 0.0f;
    float                      reward;
    
    Enemy(const glm::vec3& pos, float radius, float hp, float speed, EnemyAttribute attr, int rewardValue, std::unique_ptr<IMovement> mv)
      : hitbox{pos, radius}, 
        health(hp), 
        maxHealth(hp),
        baseSpeed(speed), 
        attribute(attr), 
        reward(rewardValue),
        movement(std::move(mv)) {}

    float speed() const {
        float currentSpeed = baseSpeed;
        if (attribute == EnemyAttribute::FAST)
            currentSpeed *= 1.5f;
        
        if (attribute == EnemyAttribute::RESISTANT)
        currentSpeed *= 0.75f; 

        if (isSlowed)
            currentSpeed *= 0.7f; // Inimigo fica 70% mais lento

        return currentSpeed;
    }

    // ATUALIZE O MÉTODO update() para controlar o tempo da lentidão
    void update(float dt) {
        if (!alive) return;
        
        if (isSlowed) {
            slowTimer -= dt;
            if (slowTimer <= 0.0f) {
                isSlowed = false;
            }
        }

        if (movement) movement->move(*this, dt);
    }
    
    void applySlow(float duration) {
        isSlowed = true;
        slowTimer = duration;
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
            e.killedByPlayer = false;                   
            e.reward = 0; 
            printf("Um inimigo chegou a base!\n"); 
            return;
        }
        float t = caminho->getTForDistance(e.distanceTraveled);
        e.hitbox.center = caminho->getPoint(t);
    }
};