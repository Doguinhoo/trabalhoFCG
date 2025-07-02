#ifndef _BEZIER_H
#define _BEZIER_H

#include <glm/vec4.hpp>
#include <glm/gtx/string_cast.hpp>
#include <vector>
#include <cmath>
#include <cstdio>
#include <iostream>

class CurvaBezier {
    std::vector<glm::vec4> pontos;
    public:
        CurvaBezier(glm::vec4*, size_t);
        glm::vec4 calcula_ponto(float);
};

#endif // #ifndef _BEZIER_HPP
