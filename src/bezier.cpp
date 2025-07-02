#include <bezier.h>
#include <glm/vec4.hpp>
#include <glm/gtx/string_cast.hpp>
#include <vector>
#include <cmath>
#include <cstdio>
#include <iostream>

CurvaBezier::CurvaBezier(glm::vec4 *pontos, size_t num_pontos) {
    this->pontos.insert(this->pontos.begin(), pontos, pontos + num_pontos);
}

glm::vec4 CurvaBezier::calcula_ponto(float t) {
    std::vector<glm::vec4> pontos_buffer;

    for (size_t i = 0; i < pontos.size(); i++) {
        pontos_buffer.push_back(pontos[i]);
    }

    while (pontos_buffer.size() > 1) {
        for (size_t i = 0; i < pontos_buffer.size() - 1; i++) {
            pontos_buffer[i] = pontos_buffer[i] + t * (pontos_buffer[i+1] - pontos_buffer[i]);
        }

        pontos_buffer.pop_back();
    }

    return pontos_buffer[0];
}