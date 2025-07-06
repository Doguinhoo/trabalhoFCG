#pragma once

#include <glm/glm.hpp>
#include <memory>
#include <vector>
#include <string>
#include <cstdio>
#include "Enemy.h"

class Tower;

template<typename T, typename... Args>
std::unique_ptr<T> make_unique(Args&&... args) {
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

struct ITargeting {
    virtual ~ITargeting() = default;
    virtual Enemy* pick(const std::vector<Enemy*>& enemies, const Tower& self) = 0;
    virtual std::string getModeName() const = 0; 
};

struct IShooting {
    virtual ~IShooting() = default;
    virtual void fire(Enemy* target, Tower& self, const std::vector<Enemy*>& all_enemies) = 0;
    virtual std::string getDamageInfo() const = 0; 
};

struct IPassiveAbility {
    virtual ~IPassiveAbility() = default;
    virtual void onRoundEnd(Tower& self, float& playerMoney) = 0;
    virtual std::string getInfo() const = 0; 
};

class Tower {
public:
    std::string blueprintName;
    std::string modelName; 
    glm::vec3 pos;
    Hitbox    hitbox;
    float     range;
    float     cooldown;
    float     timer = 0.f;
    int    targetingModeIndex = 0;
    Enemy* currentTarget = nullptr; 
    float  currentYRotation = 0.0f; 

    std::unique_ptr<ITargeting> targeting;
    std::unique_ptr<IShooting>  shooting;
    std::unique_ptr<IPassiveAbility> passiveAbility;

    Tower(const std::string& bpName, const std::string& mdlName, const glm::vec3& p, float r, float cd,
        std::unique_ptr<ITargeting> t,
        std::unique_ptr<IShooting>  s,
        std::unique_ptr<IPassiveAbility> pa);

    Hitbox rangeHitbox() const;
    void update(float dt, const std::vector<Enemy*>& enemies);
    void updateEndOfRound(float& playerMoney);
};

// Declarações de estratégias concretas (implementações em Tower.cpp)
struct NearestTarget : ITargeting { Enemy* pick(const std::vector<Enemy*>&, const Tower&) override; std::string getModeName() const override;};
struct WeakestTarget : ITargeting { Enemy* pick(const std::vector<Enemy*>&, const Tower&) override; std::string getModeName() const override;};
struct StrongestTarget : ITargeting { Enemy* pick(const std::vector<Enemy*>&, const Tower&) override; std::string getModeName() const override;};
struct FirstTarget : ITargeting { Enemy* pick(const std::vector<Enemy*>&, const Tower&) override; std::string getModeName() const override;};
struct LastTarget : ITargeting { Enemy* pick(const std::vector<Enemy*>&, const Tower&) override; std::string getModeName() const override;};
struct FlyingPriorityTarget : ITargeting { Enemy* pick(const std::vector<Enemy*>&, const Tower&) override; std::string getModeName() const override;};

struct ProjectileShot : IShooting {
    float damage, speed;
    ProjectileShot(float dmg, float spd) : damage(dmg), speed(spd) {}
    void fire(Enemy*, Tower&, const std::vector<Enemy*>&) override;
    std::string getDamageInfo() const override;
};
struct FullAoeShot : IShooting {
    float damagePerSecond;
    FullAoeShot(float dps) : damagePerSecond(dps) {}
    void fire(Enemy*, Tower&, const std::vector<Enemy*>&) override;
    std::string getDamageInfo() const override;
};

struct ConeShot : IShooting {
    float damage, coneAngle;
    ConeShot(float dmg, float angle_rad) : damage(dmg), coneAngle(angle_rad) {}
    void fire(Enemy*, Tower&, const std::vector<Enemy*>&) override;
    std::string getDamageInfo() const override;
};

struct SplashDamageShot : IShooting {
    float primaryDamage, splashDamage, splashRadius;
    SplashDamageShot(float p_dmg, float s_dmg, float s_r) : primaryDamage(p_dmg), splashDamage(s_dmg), splashRadius(s_r) {}
    void fire(Enemy*, Tower&, const std::vector<Enemy*>&) override;
    std::string getDamageInfo() const override;
};

struct IceShot : IShooting {
    float slowDuration;
    IceShot(float duration) : slowDuration(duration) {}
    void fire(Enemy*, Tower&, const std::vector<Enemy*>&) override;
    std::string getDamageInfo() const override;
};

struct GenerateIncome : IPassiveAbility {
    int incomePerRound;
    GenerateIncome(int income) : incomePerRound(income) {}
    void onRoundEnd(Tower&, float&) override;
    std::string getInfo() const override;
};

