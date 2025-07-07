#version 330 core

in vec4 position_model;

// Variáveis para acesso das imagens de textura
uniform sampler2D TextureImages[1];

uniform vec4 bbox_min;
uniform vec4 bbox_max;

// O valor de saída ("out") de um Fragment Shader é a cor final do fragmento.
out vec4 color;

#define M_PI   3.14159265358979323846
#define M_PI_2 1.57079632679489661923

void main() {
    vec4 bbox_center = (bbox_min + bbox_max) / 2.0;
    vec4 pv = position_model - bbox_center;

    float theta = atan(pv.x, pv.z);
    float phi = asin(pv.y/length(pv)); 

    float U = (theta + M_PI)/(2*M_PI);
    float V = (phi + M_PI_2)/M_PI;

    color = texture(TextureImages[0], vec2(U,V));
    // color.rgb = texcoords;
    // color.a = 1;
} 