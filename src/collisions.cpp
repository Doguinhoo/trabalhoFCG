#include "collisions.h"
#include "utils.hpp" 

// SOBRE AS OUTRAS COLISÕES:

// Colisão entre Inimigo e a Torre:
// As colisões entre torres e inimigos foram desenvolvidas nas devidas 
// estruturas, onde o cálculo se da pela intersecção de esferas
// definidas na hitbox dos inimigos e no range das torres, feito calculando
// o quadrado da distância entre os centros com o quadrado da soma dos raios,
// onde se dist^2 <= (r1+r2)^2, as esferas se intersectam.

// Selecionar torres:
// lógica principal na função glm::vec3 GetCursorWorldPosition(GLFWwindow* window)
// onde foi declarada na main pois estava tendo conflito com o glad.h
// Converte a posição do cursor do mouse para um sistema de coordenadas normalizadas,
// onde ela "desfaz" as transformações da câmera para calcular a direção de um 
// raio que sai da câmera, passa pelo cursor e continua infinitamente.
// Encontra o ponto exato onde esse raio 3D colide com o plano horizontal do chão 
// e é exatamente essa coordenada 3D que a função retorna, onde é usado para
// ver se encontra uma torre, seleciona ela, caso contrário, seleciona nenhuma torre.

// Implementação da função de checagem de colisão da câmera
bool CheckCameraCollision(
    const glm::vec4& proposed_position,
    const std::vector<std::unique_ptr<Tower>>& towers,
    float floor_height,
    float skybox_radius,
    float camera_radius)
{
    // Colisão com o chão
    if (proposed_position.y < floor_height) {
        return true; 
    }

    // Colisão com o Skybox
    if (glm::length(glm::vec3(proposed_position)) > (skybox_radius - camera_radius)) {
        return true; 
    }

    // Colisão com cada uma das torres
    for (const auto& tower : towers) {
        float min_dist_sq = pow(tower->hitbox.radius + camera_radius, 2) - 2.5f;
        if (distance(glm::vec3(proposed_position), tower->pos) < min_dist_sq) {
            return true; 
        }
    }

    // Se passou por todas as verificações, não houve colisão
    return false;
}
