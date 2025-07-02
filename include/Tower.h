// include/Tower.h
#pragma once

#include <glm/glm.hpp>
#include <memory>
#include <vector>
#include <limits>
#include "Enemy.h"

// Interface de targeting
struct ITargeting {
    virtual ~ITargeting() = default;
    virtual Enemy* pick(const std::vector<Enemy*>& enemies,
                        const struct Tower& self) = 0;
};

// Interface de shooting
struct IShooting {
    virtual ~IShooting() = default;
    virtual void fire(Enemy* target, struct Tower& self) = 0;
};

// Classe genérica de torre
class Tower {
public:
    glm::vec3 pos;
    float     range;
    float     cooldown;
    float     timer = 0.f;

    std::unique_ptr<ITargeting> targeting;
    std::unique_ptr<IShooting>  shooting;

    Tower(const glm::vec3& p, float r, float cd,
          std::unique_ptr<ITargeting> t,
          std::unique_ptr<IShooting>  s)
      : pos(p), range(r), cooldown(cd),
        targeting(std::move(t)), shooting(std::move(s))
    {}

    Hitbox rangeHitbox() const {
        return { pos, range };
    }

    void update(float dt, const std::vector<Enemy*>& enemies) {
        timer -= dt;
        if (timer <= 0.f && targeting && shooting) {
            if (Enemy* tgt = targeting->pick(enemies, *this)) {
                shooting->fire(tgt, *this);
                timer = cooldown;
            }
        }
    }
};

// Estratégia concreta de targeting
struct NearestTarget : ITargeting {
    Enemy* pick(const std::vector<Enemy*>& enemies,
                const Tower& self) override
    {
        Enemy* best = nullptr;
        float bestD2 = std::numeric_limits<float>::infinity();
        Hitbox rs = self.rangeHitbox();

        for (auto* e : enemies) {
            // se estiver fora do alcance, pula
            if (!rs.intersects(e->hitbox)) continue;

            // dentro do alcance: escolhe o mais próximo
            glm::vec3 d = e->hitbox.center - self.pos;
            float d2 = glm::dot(d,d);
            if (d2 < bestD2) {
                bestD2 = d2;
                best   = e;
            }
        }
        return best;
    }
};

struct ProjectileShot : IShooting {
    float damage, speed;
    ProjectileShot(float dmg, float spd) : damage(dmg), speed(spd) {}
    void fire(Enemy* target, Tower&) override {
        target->applyDamage(damage);
    }
};

struct AoeShot : IShooting {
    float damage, radius;
    AoeShot(float dmg, float r) : damage(dmg), radius(r) {}
    void fire(Enemy* target, Tower&) override {
        // poderia aplicar área, mas num teste simples ataca só o alvo
        target->applyDamage(damage);
    }
};