#pragma once

#include "bezier.h" 
#include <glm/glm.hpp>
#include <vector>
#include <utility>
#include <memory>

// Header dos caminhos
// Inimigos percorrem uma curva bezier
class Path {
public:
    // O construtor recebe os pontos de controle e cria a CurvaBezier internamente
    Path(const std::vector<glm::vec4>& control_points)
        : curva_interna(const_cast<glm::vec4*>(control_points.data()), control_points.size()) {}

    // Delega o cálculo do ponto
    glm::vec3 getPoint(float t) {
        return glm::vec3(curva_interna.calcula_ponto(t));
    }

    // computa o cáluclo
    void precompute(int samples = 200) {
        arcLengthTable.clear();
        totalLength = 0.0f;

        glm::vec3 prevPoint = getPoint(0);
        arcLengthTable.emplace_back(0.0f, 0.0f);

        for (int i = 1; i <= samples; ++i) {
            float t = static_cast<float>(i) / samples;
            glm::vec3 currentPoint = getPoint(t);
            totalLength += glm::distance(currentPoint, prevPoint);
            arcLengthTable.emplace_back(totalLength, t);
            prevPoint = currentPoint;
        }
    }

    float getTForDistance(float distance) const {
        if (distance <= 0) 
            return 0.0f;
        if (distance >= totalLength) 
            return 1.0f;

        for (size_t i = 1; i < arcLengthTable.size(); ++i) {
            if (arcLengthTable[i].first >= distance) {
                float dist1 = arcLengthTable[i-1].first;
                float t1 = arcLengthTable[i-1].second;
                float dist2 = arcLengthTable[i].first;
                float t2 = arcLengthTable[i].second;
                float fraction = (distance - dist1) / (dist2 - dist1);
                return glm::mix(t1, t2, fraction);
            }
        }
        return 1.0f;
    }

    float getTotalLength() const { 
        return totalLength; 
    }

    glm::vec3 getStartPoint() { 
        return getPoint(0.0f); 
    }

    glm::vec3 getEndPoint() { 
        return getPoint(1.0f); 
    }

private:
    CurvaBezier curva_interna;
    std::vector<std::pair<float, float>> arcLengthTable;
    float totalLength = 0.0f;
};