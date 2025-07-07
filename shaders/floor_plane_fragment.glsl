#version 330 core

// Atributos de fragmentos recebidos como entrada ("in") pelo Fragment Shader.
// Neste exemplo, este atributo foi gerado pelo rasterizador como a
// interpolação da posição global e a normal de cada vértice, definidas em
// "shader_vertex.glsl" e "main.cpp".
in vec4 position_world;
in vec4 normal;

// Posição do vértice atual no sistema de coordenadas local do modelo.
in vec4 position_model;

// Coordenadas de textura obtidas do arquivo OBJ (se existirem!)
in vec2 texcoords;

// Matrizes computadas no código C++ e enviadas para a GPU
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

// Variáveis para acesso das imagens de textura
uniform sampler2D TextureImages[1];

uniform vec4 light_source;
uniform vec3 light_color;
uniform vec3 ambient_color;
uniform vec3 Ka;
uniform vec3 Ks;
uniform float q;

// O valor de saída ("out") de um Fragment Shader é a cor final do fragmento.
out vec4 color;

// Constantes
const vec4 ORIGIN = vec4(0.0, 0.0, 0.0, 1.0);

void main() {
    // Obtemos a posição da câmera utilizando a inversa da matriz que define o
    // sistema de coordenadas da câmera.
    vec4 camera_position = inverse(view) * ORIGIN;

    // Normal do fragmento atual, interpolada pelo rasterizador a partir das
    // normais de cada vértice.
    vec4 n = normalize(normal);
    n.w = 0;

    #define p position_world

    // Vetor que define o sentido da câmera em relação ao ponto atual.
    vec4 v = normalize(camera_position - p); 

    vec4 l;

    if (light_source.w == 0) {
        l = normalize(light_source);
    } else {
        l = normalize(light_source - p);
    }

    vec4 h = normalize(v + l);

    // Obtemos a refletância difusa a partir da leitura da imagem TextureImages[0]
    vec2 tiled_uv = position_world.xz * 0.1; 
    vec3 Kd = texture(TextureImages[0], tiled_uv).rgb;

    // Equações de Iluminação
    vec3 lambert = Kd*light_color*max(0,dot(n,l));
    vec3 ambient = Ka*Kd*ambient_color;
    vec3 blinn_phong = Ks*light_color*pow(max(dot(n, h), 0), q);

    color.rgb = lambert + ambient + blinn_phong;

    // NOTE: Se você quiser fazer o rendering de objetos transparentes, é
    // necessário:
    // 1) Habilitar a operação de "blending" de OpenGL logo antes de realizar o
    //    desenho dos objetos transparentes, com os comandos abaixo no código C++:
    //      glEnable(GL_BLEND);
    //      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // 2) Realizar o desenho de todos objetos transparentes *após* ter desenhado
    //    todos os objetos opacos; e
    // 3) Realizar o desenho de objetos transparentes ordenados de acordo com
    //    suas distâncias para a câmera (desenhando primeiro objetos
    //    transparentes que estão mais longe da câmera).
    // Alpha default = 1 = 100% opaco = 0% transparente
    color.a = 1;

    // Cor final com correção gamma, considerando monitor sRGB.
    // Veja https://en.wikipedia.org/w/index.php?title=Gamma_correction&oldid=751281772#Windows.2C_Mac.2C_sRGB_and_TV.2Fvideo_standard_gammas
    color.rgb = pow(color.rgb, vec3(1.0,1.0,1.0)/2.2);
}