#version 330 core

in vec4 position_model;

// Variáveis para acesso das imagens de textura
uniform samplerCube TextureImages[1];

// O valor de saída ("out") de um Fragment Shader é a cor final do fragmento.
out vec4 color;

void main() {
    color = texture(TextureImages[0], vec3(position_model.x, position_model.y, position_model.z));
} 