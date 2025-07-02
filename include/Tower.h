// include/Tower.h
#pragma once

#include <glm/glm.hpp>
#include <glm/gtx/vector_angle.hpp>
#include <memory>
#include <vector>
#include <limits>
#include "Enemy.h"

// Fwd declaration
class Tower;

// Interface de targeting
struct ITargeting {
    virtual ~ITargeting() = default;
    virtual Enemy* pick(const std::vector<Enemy*>& enemies,
                        const Tower& self) = 0;
};

// Interface de shooting (ASSINATURA MODIFICADA)
struct IShooting {
    virtual ~IShooting() = default;
    virtual void fire(Enemy* target, Tower& self, const std::vector<Enemy*>& all_enemies) = 0;
};

// Classe genérica de torre
class Tower {
public:
    std::string blueprintName; // <-- ADICIONADO
    glm::vec3 pos;
    float     range;
    float     cooldown;
    float     timer = 0.f;

    std::unique_ptr<ITargeting> targeting;
    std::unique_ptr<IShooting>  shooting;

    Tower(const std::string& bpName, const glm::vec3& p, float r, float cd,
          std::unique_ptr<ITargeting> t,
          std::unique_ptr<IShooting>  s)
      : blueprintName(bpName), pos(p), range(r), cooldown(cd),
        targeting(std::move(t)), shooting(std::move(s))
    {}

    Hitbox rangeHitbox() const {
        return { pos, range };
    }

    void update(float dt, const std::vector<Enemy*>& enemies) {
        timer -= dt;
        if (timer <= 0.f && targeting && shooting) {
            if (Enemy* tgt = targeting->pick(enemies, *this)) {
                shooting->fire(tgt, *this, enemies); // <-- MODIFICADO
                timer = cooldown;
            }
        }
    }
};

// =======================================================
// ESTRATÉGIAS DE TARGETING
// =======================================================

struct NearestTarget : ITargeting {
    Enemy* pick(const std::vector<Enemy*>& enemies, const Tower& self) override;
};

struct WeakestTarget : ITargeting {
    Enemy* pick(const std::vector<Enemy*>& enemies, const Tower& self) override;
};

struct StrongestTarget : ITargeting {
    Enemy* pick(const std::vector<Enemy*>& enemies, const Tower& self) override;
};

struct FirstTarget : ITargeting {
    Enemy* pick(const std::vector<Enemy*>& enemies, const Tower& self) override;
};

struct LastTarget : ITargeting {
    Enemy* pick(const std::vector<Enemy*>& enemies, const Tower& self) override;
};

struct FlyingPriorityTarget : ITargeting {
    Enemy* pick(const std::vector<Enemy*>& enemies, const Tower& self) override;
};


// =======================================================
// ESTRATÉGIAS DE SHOOTING
// =======================================================

struct ProjectileShot : IShooting {
    float damage, speed;
    ProjectileShot(float dmg, float spd) : damage(dmg), speed(spd) {}
    void fire(Enemy* target, Tower&, const std::vector<Enemy*>&) override;
};

struct FullAoeShot : IShooting {
    float damagePerSecond;
    FullAoeShot(float dps) : damagePerSecond(dps) {}
    void fire(Enemy* target, Tower& self, const std::vector<Enemy*>& all_enemies) override;
};

struct ConeShot : IShooting {
    float damage;
    float coneAngle; // Em radianos
    ConeShot(float dmg, float angle_rad) : damage(dmg), coneAngle(angle_rad) {}
    void fire(Enemy* target, Tower& self, const std::vector<Enemy*>& all_enemies) override;
};

struct SplashDamageShot : IShooting {
    float primaryDamage;
    float splashDamage;
    float splashRadius;
    SplashDamageShot(float p_dmg, float s_dmg, float s_radius)
        : primaryDamage(p_dmg), splashDamage(s_dmg), splashRadius(s_radius) {}
    void fire(Enemy* target, Tower& self, const std::vector<Enemy*>& all_enemies) override;
};