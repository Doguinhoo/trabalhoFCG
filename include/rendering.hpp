#ifndef RENDERING_HPP
#define RENDERING_HPP

#include <glad/glad.h>
#include <tiny_obj_loader.h>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

#include <vector>

const size_t MAX_TEXTURES = 1;

struct ObjModel {
    tinyobj::attrib_t                 attrib;
    std::vector<tinyobj::shape_t>     shapes;
    std::vector<tinyobj::material_t>  materials;

    // Este construtor lê o modelo de um arquivo utilizando a biblioteca tinyobjloader.
    // Veja: https://github.com/syoyo/tinyobjloader
    ObjModel(const char* filename, const char* basepath = NULL, bool triangulate = true);
};

class Shape {
    ObjModel& objectModel;
    const char *shape_name;

    size_t first_index; // Índice do primeiro vértice dentro do vetor indices[] definido em BuildTrianglesAndAddToVirtualScene()
    size_t num_indices; // Número de índices do objeto dentro do vetor indices[] definido em BuildTrianglesAndAddToVirtualScene()
    GLenum rendering_mode; // Modo de rasterização (GL_TRIANGLES, GL_TRIANGLE_STRIP, etc.)
    GLuint vertex_array_object_id; // ID do VAO onde estão armazenados os atributos do modelo
    glm::vec3 bbox_min; // Axis-Aligned Bounding Box do objeto
    glm::vec3 bbox_max;

    void buildTriangles();

    public:
        Shape(ObjModel& objectModel, const char *shape_name);
        glm::vec3 get_bbox_min();
        glm::vec3 get_bbox_max();
        void draw();
};

// Definimos uma estrutura que armazenará dados necessários para renderizar
// cada objeto da cena virtual.
class SceneObject {
    Shape& shapeObject;
    GLuint gpuProgram; // Programa de GPU usado para renderizar o objeto
    std::vector<GLint> textureImages;
    glm::vec3 Ka;
    glm::vec3 Ks;
    float q;

    public:
        SceneObject(
            Shape& shapeObject,
            const char *vertex_shader_file_name, const char *fragment_shader_file_name,
            std::vector<GLint> textureImages,
            glm::vec3 Ka, glm::vec3 Ks, float q);
        SceneObject(
            Shape& shapeObject, 
            GLuint vertex_shader_id, GLuint fragment_shader_id,
            std::vector<GLint> textureImages,
            glm::vec3 Ka, glm::vec3 Ks, float q);
        SceneObject(
            Shape& shapeObject, 
            GLuint gpuProgram,
            std::vector<GLint> textureImages,
            glm::vec3 Ka, glm::vec3 Ks, float q);
        void draw(
            glm::mat4x4 model, glm::mat4x4 view, glm::mat4x4 projection,
            glm::vec4 light_source, glm::vec3 light_color, glm::vec3 ambient_color);
};

typedef std::map<std::string, SceneObject> VirtualScene;

GLuint CreateGpuProgram(GLuint vertex_shader_id, GLuint fragment_shader_id); // Cria um programa de GPU
GLuint CreateGpuProgramFromFiles(const char *vertex_shader_file_name, const char *fragment_shader_file_name); // Carrega os shaders de vértice e fragmento, criando um programa de GPU
void LoadTextureImage(const char* filename, GLuint number);
void ComputeNormals(ObjModel* model); // Computa normais de um ObjModel, caso não existam.
GLuint LoadShader_Vertex(const char* filename);
GLuint LoadShader_Fragment(const char* filename);

#endif // RENDERING_HPP