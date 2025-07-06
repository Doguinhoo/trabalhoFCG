#version 330 core

// Atributos de vértice recebidos como entrada ("in") pelo Vertex Shader.
// Veja a função BuildTrianglesAndAddToVirtualScene() em "main.cpp".
layout (location = 0) in vec4 model_coefficients;
layout (location = 1) in vec4 normal_coefficients;
layout (location = 2) in vec2 texture_coefficients;

// Matrizes computadas no código C++ e enviadas para a GPU
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform vec4 light_source;
uniform vec3 light_color;
uniform vec3 ambient_color;
uniform vec3 Ka;
uniform vec3 Ks;
uniform float q;

// Atributos de vértice que serão gerados como saída ("out") pelo Vertex Shader.
// ** Estes serão interpolados pelo rasterizador! ** gerando, assim, valores
// para cada fragmento, os quais serão recebidos como entrada pelo Fragment
// Shader. Veja o arquivo "shader_fragment.glsl".
out vec4 position_world;
out vec4 position_model;
out vec4 normal;
out vec2 texcoords;
out vec3 gouraud;

const vec4 ORIGIN = vec4(0.0,0.0,0.0,1.0);

void main() {
    // A variável gl_Position define a posição final de cada vértice
    // OBRIGATORIAMENTE em "normalized device coordinates" (NDC), onde cada
    // coeficiente estará entre -1 e 1 após divisão por w.
    // Veja {+NDC2+}.
    //
    // O código em "main.cpp" define os vértices dos modelos em coordenadas
    // locais de cada modelo (array model_coefficients). Abaixo, utilizamos
    // operações de modelagem, definição da câmera, e projeção, para computar
    // as coordenadas finais em NDC (variável gl_Position). Após a execução
    // deste Vertex Shader, a placa de vídeo (GPU) fará a divisão por W. Veja
    // slides 41-67 e 69-86 do documento Aula_09_Projecoes.pdf.

    gl_Position = projection * view * model * model_coefficients;

    // Posição do vértice atual no sistema de coordenadas global (World).
    position_world = model * model_coefficients;
    #define p position_world

    // Posição do vértice atual no sistema de coordenadas local do modelo.
    position_model = model_coefficients;

    #define n normal
    // Normal do vértice atual no sistema de coordenadas global (World).
    // Veja slides 123-151 do documento Aula_07_Transformacoes_Geometricas_3D.pdf.
    n = normalize(inverse(transpose(model)) * normal_coefficients);
    n.w = 0.0;
    
    // Coordenadas de textura obtidas do arquivo OBJ (se existirem!)
    texcoords = texture_coefficients;

    vec4 l;

    if (light_source.w == 0) {
        l = normalize(light_source);
    } else {
        l = normalize(light_source - p);
        
    }

    // Termo difuso utilizando a lei dos cossenos de Lambert
    vec3 lambert_diffuse_term = light_color*max(0, dot(n, l));

    // Termo especular utilizando o modelo de iluminação de Phong
    vec4 camera_position = inverse(view) * ORIGIN;
    vec4 v = normalize(camera_position - p);
    vec4 h = normalize(v + l);
    vec3 phong_specular_term  = Ks*light_color*pow(max(dot(n, h), 0.0), q);

    // Termo ambiente
    vec3 ambient_term = Ka*ambient_color;

    gouraud.rgb = lambert_diffuse_term + ambient_term + phong_specular_term;
}

