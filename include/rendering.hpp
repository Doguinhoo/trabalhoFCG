#ifndef RENDERING_HPP
#define RENDERING_HPP

#include <glad/glad.h>
#include <tiny_obj_loader.h>
#include <glm/vec3.hpp>

#include <vector>

struct ObjModel {
    tinyobj::attrib_t                 attrib;
    std::vector<tinyobj::shape_t>     shapes;
    std::vector<tinyobj::material_t>  materials;

    // Este construtor lê o modelo de um arquivo utilizando a biblioteca tinyobjloader.
    // Veja: https://github.com/syoyo/tinyobjloader
    ObjModel(const char* filename, const char* basepath = NULL, bool triangulate = true);
};

// Definimos uma estrutura que armazenará dados necessários para renderizar
// cada objeto da cena virtual.
class SceneObject {
    std::string  name;        // Nome do objeto
    size_t       first_index; // Índice do primeiro vértice dentro do vetor indices[] definido em BuildTrianglesAndAddToVirtualScene()
    GLuint       GpuProgram; // Programa de gpu usado pra renderizar o objeto
    size_t       num_indices; // Número de índices do objeto dentro do vetor indices[] definido em BuildTrianglesAndAddToVirtualScene()
    GLenum       rendering_mode; // Modo de rasterização (GL_TRIANGLES, GL_TRIANGLE_STRIP, etc.)
    GLuint       vertex_array_object_id; // ID do VAO onde estão armazenados os atributos do modelo
    glm::vec3    bbox_min; // Axis-Aligned Bounding Box do objeto
    glm::vec3    bbox_max;

    public:
        SceneObject(ObjModel&, const char *shape_name);
        void draw();
};

typedef std::map<std::string, SceneObject> VirtualScene;

GLuint CreateGpuProgram(GLuint vertex_shader_id, GLuint fragment_shader_id); // Cria um programa de GPU
GLuint CreateGpuProgramFromFiles(const char *vertex_shader_file_name, const char *fragment_shader_file_name); // Carrega os shaders de vértice e fragmento, criando um programa de GPU
void LoadTextureImage(const char* filename, int number);
void ComputeNormals(ObjModel* model); // Computa normais de um ObjModel, caso não existam.

#endif // RENDERING_HPP