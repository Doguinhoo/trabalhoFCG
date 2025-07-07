#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <memory>
#include "Tower.h"

// Verifica se a posição proposta para a câmera colide com algum elemento do cenário.
bool CheckCameraCollision(
    const glm::vec4& proposed_position,
    const std::vector<std::unique_ptr<Tower>>& towers,
    float floor_height,
    float ceiling_height,
    float skybox_radius,
    float camera_radius
);
