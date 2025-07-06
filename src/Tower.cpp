#define _USE_MATH_DEFINES
#include <cmath>
#include "Tower.h"
#include <glm/gtx/vector_angle.hpp>
#include <limits>

Tower::Tower(const std::string& bpName, const std::string& mdlName, const glm::vec3& p, float r, float cd,
             std::unique_ptr<ITargeting> t, std::unique_ptr<IShooting> s, std::unique_ptr<IPassiveAbility> pa)
    // A lista de inicialização agora cria a hitbox com um raio padrão de 1.0f
    : blueprintName(bpName),
      modelName(mdlName),
      pos(p),
      hitbox({p, 1.0f}),
      range(r),
      cooldown(cd),
      targeting(std::move(t)),
      shooting(std::move(s)),
      passiveAbility(std::move(pa))
{
    // O corpo do construtor pode ficar vazio
}

Hitbox Tower::rangeHitbox() const {
    return { pos, range };
}

void Tower::update(float dt, const std::vector<Enemy*>& enemies) {
    if (!targeting) return; // Torres passivas (Farm) não fazem nada aqui.

    //  A torre usa sua estratégia para escolher um alvo da lista de inimigos.
    currentTarget = targeting->pick(enemies, *this);

    //  Se um alvo foi encontrado, a torre se vira para ele.
    if (currentTarget != nullptr)
    {
        // Calcula o vetor de direção da torre para o inimigo no plano (XZ)
        glm::vec3 direction = currentTarget->hitbox.center - this->pos;
        float targetYRotation = atan2(direction.x, direction.z); 
        
        // Interpola suavemente da rotação atual para a rotação do alvo.
        currentYRotation += (targetYRotation - currentYRotation) * 10.0f * dt;
    }

    // Lógica de tiro (só atira se tiver um alvo e o cooldown estiver zerado)
    timer -= dt;
    if (shooting && currentTarget != nullptr && timer <= 0.f)
    {
        shooting->fire(currentTarget, *this, enemies);
        timer = cooldown; 
    }
}

void Tower::updateEndOfRound(float& playerMoney) {
    if (passiveAbility) {
        passiveAbility->onRoundEnd(*this, playerMoney);
    }
}


// --- Implementações de ITargeting ---

Enemy* NearestTarget::pick(const std::vector<Enemy*>& enemies, const Tower& self) {
    Enemy* best = nullptr;
    float bestD2 = std::numeric_limits<float>::infinity();
    Hitbox rs = self.rangeHitbox();
    for (auto* e : enemies) {
        if (!e->alive || !rs.intersects(e->hitbox)) continue;
        if (e->attribute == EnemyAttribute::FLYING && !self.canTargetFlying) {
            continue;
        }
        glm::vec3 d = e->hitbox.center - self.pos;
        float d2 = glm::dot(d,d);
        if (d2 < bestD2) {
            bestD2 = d2;
            best = e;
        }
    }
    return best;
}

std::string NearestTarget::getModeName() const { return "Mais Proximo"; }

Enemy* WeakestTarget::pick(const std::vector<Enemy*>& enemies, const Tower& self) {
    Enemy* weakest = nullptr;
    float lowestHealth = std::numeric_limits<float>::infinity();
    Hitbox range = self.rangeHitbox();
    for (auto* e : enemies) {
        if (!e->alive || !range.intersects(e->hitbox)) continue;
        if (e->health < lowestHealth) {
            lowestHealth = e->health;
            weakest = e;
        }
    }
    return weakest;
}

std::string WeakestTarget::getModeName() const { return "Mais Fraco"; }

Enemy* StrongestTarget::pick(const std::vector<Enemy*>& enemies, const Tower& self) {
    Enemy* strongest = nullptr;
    float highestHealth = 0.0f;
    Hitbox range = self.rangeHitbox();
    for (auto* e : enemies) {
        if (!e->alive || !range.intersects(e->hitbox)) continue;
        if (e->health > highestHealth) {
            highestHealth = e->health;
            strongest = e;
        }
    }
    return strongest;
}

std::string StrongestTarget::getModeName() const { return "Mais Forte"; }

Enemy* FirstTarget::pick(const std::vector<Enemy*>& enemies, const Tower& self) {
    Enemy* first = nullptr;
    float maxDistance = -1.0f;
    Hitbox range = self.rangeHitbox();
    for (auto* e : enemies) {
        if (!e->alive || !range.intersects(e->hitbox)) continue;
        if (e->distanceTraveled > maxDistance) {
            maxDistance = e->distanceTraveled;
            first = e;
        }
    }
    return first;
}

std::string FirstTarget::getModeName() const { return "Primeiro"; }

Enemy* LastTarget::pick(const std::vector<Enemy*>& enemies, const Tower& self) {
    Enemy* last = nullptr;
    float minDistance = std::numeric_limits<float>::infinity();
    Hitbox range = self.rangeHitbox();
    for (auto* e : enemies) {
        if (!e->alive || !range.intersects(e->hitbox)) continue;
        if (e->distanceTraveled < minDistance) {
            minDistance = e->distanceTraveled;
            last = e;
        }
    }
    return last;
}

std::string LastTarget::getModeName() const { return "Ultimo"; }

Enemy* FlyingPriorityTarget::pick(const std::vector<Enemy*>& enemies, const Tower& self) {
    Enemy* bestFlying = nullptr;
    float maxDistance = -1.0f;
    Hitbox range = self.rangeHitbox();
    for (auto* e : enemies) {
        if (!e->alive || !range.intersects(e->hitbox)) continue;
        if (e->attribute == EnemyAttribute::FLYING) {
            if (e->distanceTraveled > maxDistance) {
                maxDistance = e->distanceTraveled;
                bestFlying = e;
            }
        }
    }
    if (bestFlying) return bestFlying;
    
    // Fallback para o inimigo mais avançado se nenhum voador for encontrado
    FirstTarget fallback;
    return fallback.pick(enemies, self);
}

std::string FlyingPriorityTarget::getModeName() const { return "Voador"; }

// --- Implementações de IShooting ---

void ProjectileShot::fire(Enemy* target, Tower&, const std::vector<Enemy*>&) {
    if (target) target->applyDamage(damage);
}


void FullAoeShot::fire(Enemy*, Tower& self, const std::vector<Enemy*>& all_enemies) {
    Hitbox range = self.rangeHitbox();
    float damage_this_tick = damagePerSecond * self.cooldown;
    for (auto* enemy : all_enemies) {
        if (enemy && enemy->alive && range.intersects(enemy->hitbox)) {
            enemy->applyDamage(damage_this_tick);
        }
    }
}

void ConeShot::fire(Enemy* target, Tower& self, const std::vector<Enemy*>& all_enemies) {
    if (!target) return;
    Hitbox range = self.rangeHitbox();
    glm::vec3 towerToTargetDir = glm::normalize(target->hitbox.center - self.pos);
    for (auto* enemy : all_enemies) {
        if (!enemy || !enemy->alive || !range.intersects(enemy->hitbox)) continue;
        glm::vec3 towerToEnemyDir = glm::normalize(enemy->hitbox.center - self.pos);
        float angle = glm::angle(towerToTargetDir, towerToEnemyDir);
        if (angle <= coneAngle / 2.0f) {
            enemy->applyDamage(damage);
        }
    }
}

void SplashDamageShot::fire(Enemy* target, Tower&, const std::vector<Enemy*>& all_enemies) {
    if (!target) return;
    target->applyDamage(primaryDamage);
    Hitbox splashZone = { target->hitbox.center, splashRadius };
    for (auto* enemy : all_enemies) {
        if (!enemy || !enemy->alive || enemy == target) continue;
        if (splashZone.intersects(enemy->hitbox)) {
            enemy->applyDamage(splashDamage);
        }
    }
}

void IceShot::fire(Enemy* target, Tower&, const std::vector<Enemy*>&) {
    if (target) {
        target->applySlow(slowDuration);
    }
}

std::string IceShot::getDamageInfo() const {
    return "Lentidao";
}

std::string ProjectileShot::getDamageInfo() const {
    return std::to_string((int)damage);
}

std::string SplashDamageShot::getDamageInfo() const {
    char buffer[50];
    snprintf(buffer, sizeof(buffer), "%d (+%d)", (int)primaryDamage, (int)splashDamage);
    return std::string(buffer);
}

std::string ConeShot::getDamageInfo() const {
    return std::to_string((int)damage);
}

std::string FullAoeShot::getDamageInfo() const {
    char buffer[50];
    snprintf(buffer, sizeof(buffer), "%d/s", (int)damagePerSecond);
    return std::string(buffer);
}

// --- Implementações de IPassiveAbility ---

void GenerateIncome::onRoundEnd(Tower& self, float& playerMoney) {
    playerMoney += incomePerRound;
    printf("Farm em (%.1f, %.1f) gerou +%d de dinheiro.\n", self.pos.x, self.pos.z, incomePerRound);
}

std::string GenerateIncome::getInfo() const {
    char buffer[50];
    snprintf(buffer, sizeof(buffer), "+%d / round", incomePerRound);
    return std::string(buffer);
}