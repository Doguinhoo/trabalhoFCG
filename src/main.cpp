//     Universidade Federal do Rio Grande do Sul
//             Instituto de Informática
//       Departamento de Informática Aplicada
//
//    INF01047 Fundamentos de Computação Gráfica
//               Prof. Eduardo Gastal
//
//                   LABORATÓRIO 5
//

// Arquivos "headers" padrões de C podem ser incluídos em um
// programa C++, sendo necessário somente adicionar o caractere
// "c" antes de seu nome, e remover o sufixo ".h". Exemplo:
//    #include <stdio.h> // Em C
//  vira
//    #include <cstdio> // Em C++
//
#include <cmath>
#include <cstdio>
#include <cstdlib>

// Headers abaixo são específicos de C++
using namespace std;
#include <iostream>
#include <map>
#include <stack>
#include <string>
#include <vector>
#include <limits>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <algorithm>

// Headers das bibliotecas OpenGL
#include <glad/glad.h>   // Criação de contexto OpenGL 3.3
#include <GLFW/glfw3.h>  // Criação de janelas do sistema operacional

// Headers da biblioteca GLM: criação de matrizes e vetores.
#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>
#include <glm/gtc/type_ptr.hpp>

// Headers da biblioteca para carregar modelos obj
#include <tiny_obj_loader.h>
#include <stb_image.h>

// Headers locais, definidos na pasta "include/"
#include "utils.h"
#include "matrices.h"

// Headers das estruturas
#include "EnemyManager.h"
#include "Enemy.h"
#include "Shop.h"
#include "TowerBlueprint.h"
#include "Tower.h"
#include "Path.h"

// Estrutura que representa um modelo geométrico carregado a partir de um
// arquivo ".obj". Veja https://en.wikipedia.org/wiki/Wavefront_.obj_file .
struct ObjModel
{
    tinyobj::attrib_t                 attrib;
    std::vector<tinyobj::shape_t>     shapes;
    std::vector<tinyobj::material_t>  materials;

    // Este construtor lê o modelo de um arquivo utilizando a biblioteca tinyobjloader.
    // Veja: https://github.com/syoyo/tinyobjloader
    ObjModel(const char* filename, const char* basepath = NULL, bool triangulate = true)
    {
        printf("Carregando objetos do arquivo \"%s\"...\n", filename);

        // Se basepath == NULL, então setamos basepath como o dirname do
        // filename, para que os arquivos MTL sejam corretamente carregados caso
        // estejam no mesmo diretório dos arquivos OBJ.
        std::string fullpath(filename);
        std::string dirname;
        if (basepath == NULL)
        {
            auto i = fullpath.find_last_of("/");
            if (i != std::string::npos)
            {
                dirname = fullpath.substr(0, i+1);
                basepath = dirname.c_str();
            }
        }

        std::string warn;
        std::string err;
        bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filename, basepath, triangulate);

        if (!err.empty())
            fprintf(stderr, "\n%s\n", err.c_str());

        if (!ret)
            throw std::runtime_error("Erro ao carregar modelo.");

        for (size_t shape = 0; shape < shapes.size(); ++shape)
        {
            if (shapes[shape].name.empty())
            {
                fprintf(stderr,
                        "*********************************************\n"
                        "Erro: Objeto sem nome dentro do arquivo '%s'.\n"
                        "Veja https://www.inf.ufrgs.br/~eslgastal/fcg-faq-etc.html#Modelos-3D-no-formato-OBJ .\n"
                        "*********************************************\n",
                    filename);
                throw std::runtime_error("Objeto sem nome.");
            }
            printf("- Objeto '%s'\n", shapes[shape].name.c_str());
        }

        printf("OK.\n");
    }
};


// Declaração de funções utilizadas para pilha de matrizes de modelagem.
void PushMatrix(glm::mat4 M);
void PopMatrix(glm::mat4& M);

// Declaração de várias funções utilizadas em main().  Essas estão definidas
// logo após a definição de main() neste arquivo.
void BuildTrianglesAndAddToVirtualScene(ObjModel*); // Constrói representação de um ObjModel como malha de triângulos para renderização
void ComputeNormals(ObjModel* model); // Computa normais de um ObjModel, caso não existam.
void LoadShadersFromFiles(); // Carrega os shaders de vértice e fragmento, criando um programa de GPU
void LoadTextureImage(const char* filename); // Função que carrega imagens de textura
void DrawVirtualObject(const char* object_name); // Desenha um objeto armazenado em g_VirtualScene
GLuint LoadShader_Vertex(const char* filename);   // Carrega um vertex shader
GLuint LoadShader_Fragment(const char* filename); // Carrega um fragment shader
void LoadShader(const char* filename, GLuint shader_id); // Função utilizada pelas duas acima
void LoadHUDShader();
GLuint CreateGpuProgram(GLuint vertex_shader_id, GLuint fragment_shader_id); // Cria um programa de GPU
void PrintObjModelInfo(ObjModel*); // Função para debugging

// Declaração de funções auxiliares para renderizar texto dentro da janela
// OpenGL. Estas funções estão definidas no arquivo "textrendering.cpp".
void TextRendering_Init();
float TextRendering_LineHeight(GLFWwindow* window);
float TextRendering_CharWidth(GLFWwindow* window);
void TextRendering_PrintString(GLFWwindow* window, const std::string &str, float x, float y, float scale = 1.0f);
void TextRendering_PrintMatrix(GLFWwindow* window, glm::mat4 M, float x, float y, float scale = 1.0f);
void TextRendering_PrintVector(GLFWwindow* window, glm::vec4 v, float x, float y, float scale = 1.0f);
void TextRendering_PrintMatrixVectorProduct(GLFWwindow* window, glm::mat4 M, glm::vec4 v, float x, float y, float scale = 1.0f);
void TextRendering_PrintMatrixVectorProductMoreDigits(GLFWwindow* window, glm::mat4 M, glm::vec4 v, float x, float y, float scale = 1.0f);
void TextRendering_PrintMatrixVectorProductDivW(GLFWwindow* window, glm::mat4 M, glm::vec4 v, float x, float y, float scale = 1.0f);

// Funções abaixo renderizam como texto na janela OpenGL algumas matrizes e
// outras informações do programa. Definidas após main().
void TextRendering_ShowModelViewProjection(GLFWwindow* window, glm::mat4 projection, glm::mat4 view, glm::mat4 model, glm::vec4 p_model);
void TextRendering_ShowEulerAngles(GLFWwindow* window);
void TextRendering_ShowProjection(GLFWwindow* window);
void TextRendering_ShowFramesPerSecond(GLFWwindow* window);

// Funções callback para comunicação com o sistema operacional e interação do
// usuário. Veja mais comentários nas definições das mesmas, abaixo.
void FramebufferSizeCallback(GLFWwindow* window, int width, int height);
void ErrorCallback(int error, const char* description);
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode);
void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void CursorPosCallback(GLFWwindow* window, double xpos, double ypos);
void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset);

// Definimos uma estrutura que armazenará dados necessários para renderizar
// cada objeto da cena virtual.
struct SceneObject
{
    std::string  name;        // Nome do objeto
    size_t       first_index; // Índice do primeiro vértice dentro do vetor indices[] definido em BuildTrianglesAndAddToVirtualScene()
    size_t       num_indices; // Número de índices do objeto dentro do vetor indices[] definido em BuildTrianglesAndAddToVirtualScene()
    GLenum       rendering_mode; // Modo de rasterização (GL_TRIANGLES, GL_TRIANGLE_STRIP, etc.)
    GLuint       vertex_array_object_id; // ID do VAO onde estão armazenados os atributos do modelo
    glm::vec3    bbox_min; // Axis-Aligned Bounding Box do objeto
    glm::vec3    bbox_max;
};

// Abaixo definimos variáveis globais utilizadas em várias funções do código.

// A cena virtual é uma lista de objetos nomeados, guardados em um dicionário
// (map).  Veja dentro da função BuildTrianglesAndAddToVirtualScene() como que são incluídos
// objetos dentro da variável g_VirtualScene, e veja na função main() como
// estes são acessados.
std::map<std::string, SceneObject> g_VirtualScene;

// Pilha que guardará as matrizes de modelagem.
std::stack<glm::mat4>  g_MatrixStack;

// Razão de proporção da janela (largura/altura). Veja função FramebufferSizeCallback().
float g_ScreenRatio = 1.0f;

// Ângulos de Euler que controlam a rotação de um dos cubos da cena virtual
float g_AngleX = 0.0f;
float g_AngleY = 0.0f;
float g_AngleZ = 0.0f;

// "g_LeftMouseButtonPressed = true" se o usuário está com o botão esquerdo do mouse
// pressionado no momento atual. Veja função MouseButtonCallback().
bool g_LeftMouseButtonPressed = false;
bool g_RightMouseButtonPressed = false; // Análogo para botão direito do mouse
bool g_MiddleMouseButtonPressed = false; // Análogo para botão do meio do mouse

// Variáveis que definem a câmera em coordenadas esféricas, controladas pelo
// usuário através do mouse (veja função CursorPosCallback()). A posição
// efetiva da câmera é calculada dentro da função main(), dentro do loop de
// renderização.
float g_CameraTheta = 0.0f; // Ângulo no plano ZX em relação ao eixo Z
float g_CameraPhi = 0.0f;   // Ângulo em relação ao eixo Y
float g_CameraDistance = 3.5f; // Distância da câmera para a origem

// Variáveis que controlam rotação do antebraço
float g_ForearmAngleZ = 0.0f;
float g_ForearmAngleX = 0.0f;

// Variáveis que controlam translação do torso
float g_TorsoPositionX = 0.0f;
float g_TorsoPositionY = 0.0f;

// Variável que controla o tipo de projeção utilizada: perspectiva ou ortográfica.
bool g_UsePerspectiveProjection = true;

// Variável que controla se o texto informativo será mostrado na tela.
bool g_ShowInfoText = true;

// Variáveis que definem um programa de GPU (shaders). Veja função LoadShadersFromFiles().
GLuint g_GpuProgramID = 0;
GLint g_model_uniform;
GLint g_view_uniform;
GLint g_projection_uniform;
GLint g_object_id_uniform;
GLint g_bbox_min_uniform;
GLint g_bbox_max_uniform;

// Número de texturas carregadas pela função LoadTextureImage()
GLuint g_NumLoadedTextures = 0;

// pos inicial
glm::vec4 g_camera_position_c  = glm::vec4(0.0f,1.0f,3.5f,1.0f); 
float prev_time = (float)glfwGetTime();
float delta_t;


// velocidade da camera
float speed = 2.5f;

// Teclas de movimentação
bool press_W = false;
bool press_S = false;
bool press_D = false;
bool press_A = false;

// ===============================================
// VARIÁVEIS DE ESTADO DO JOGO
// ===============================================
EnemyManager g_enemyManager;
Shop g_shop;
std::vector<std::unique_ptr<Tower>> g_towers;
float g_playerMoney = 500.0f;
std::shared_ptr<Path> g_enemyPath;
double g_cursor_x = 0.0;
double g_cursor_y = 0.0;
int   g_currentRound = 0;
const int g_totalRounds = 20;
bool  g_isRoundActive = false;
float g_intermissionTimer = 10.0f;
int g_enemiesToSpawnForRound = 0;
Tower* g_selectedTower = nullptr;
glm::mat4 g_view_matrix;
glm::mat4 g_projection_matrix;
bool g_isTowerInfoWindowOpen = false; 
enum class CameraMode { ORBIT, TOWER_FOV };
CameraMode g_cameraMode = CameraMode::ORBIT;

// Movimento “dummy” que não faz nada
struct DummyMovement : IMovement {
    void move(Enemy& , float) override {}
};

static std::unique_ptr<ITargeting> makeFirst() { 
    return std::unique_ptr<FirstTarget>(new FirstTarget()); 
}

static std::unique_ptr<ITargeting> makeStrongest() { 
    return std::unique_ptr<StrongestTarget>(new StrongestTarget()); 
}

static std::unique_ptr<ITargeting> makeNearest() { 
    return std::unique_ptr<NearestTarget>(new NearestTarget());
}

static std::unique_ptr<ITargeting> makeWeakest() { 
    return std::unique_ptr<WeakestTarget>(new WeakestTarget());
}

static std::unique_ptr<ITargeting> makeLast() { 
    return std::unique_ptr<LastTarget>(new LastTarget());
}

static std::unique_ptr<ITargeting> makeFlyingPriority() { 
    return std::unique_ptr<FlyingPriorityTarget>(new FlyingPriorityTarget());
}

std::vector<std::function<std::unique_ptr<ITargeting>()>> g_targetingFactories;


void SetupGame()
{
    printf("Configurando o jogo...\n");

    // --- cminho dos inimigos é uma curva bezier ---
    std::vector<glm::vec4> controlPoints = {
        {-20.0f, -1.0f, 0.0f, 1.0f}, {-5.0f, -1.0f, 15.0f, 1.0f},
        {5.0f,  -1.0f, -15.0f, 1.0f}, {20.0f, -1.0f, 0.0f, 1.0f}
    };
    g_enemyPath = std::shared_ptr<Path>(new Path(controlPoints));
    g_enemyPath->precompute();

    g_targetingFactories.clear(); 
    
    g_targetingFactories.push_back(makeFirst);
    g_targetingFactories.push_back(makeLast);
    g_targetingFactories.push_back(makeStrongest);
    g_targetingFactories.push_back(makeWeakest); 
    g_targetingFactories.push_back(makeNearest);
    g_targetingFactories.push_back(makeFlyingPriority);
    // --- Torre de Canhão ---
    TowerBlueprint cannonV1_bp;
    cannonV1_bp.name = "CannonTower_V1";
    cannonV1_bp.modelName = "the_cannon_tower";
    cannonV1_bp.cost = 100;
    cannonV1_bp.range = 8.0f;
    cannonV1_bp.cooldown = 0.8f;
    cannonV1_bp.targetingFactory = makeFirst;
    cannonV1_bp.shootingFactory = [](){ return std::unique_ptr<ProjectileShot>(new ProjectileShot(25, 0)); };
    cannonV1_bp.passiveFactory = nullptr;
    cannonV1_bp.upgradeCost = 150;
    cannonV1_bp.nextUpgradeName = "CannonTower_V2";
    g_shop.registerTower(cannonV1_bp);

    TowerBlueprint cannonV2_bp;
    cannonV2_bp.name = "CannonTower_V2";
    cannonV2_bp.modelName = "the_cannon_tower";
    cannonV2_bp.cost = 0;
    cannonV2_bp.range = 9.5f;
    cannonV2_bp.cooldown = 0.6f;
    cannonV2_bp.targetingFactory = makeFirst;
    cannonV2_bp.shootingFactory = [](){ return std::unique_ptr<ProjectileShot>(new ProjectileShot(45, 0)); };
    cannonV2_bp.passiveFactory = nullptr; 
    g_shop.registerTower(cannonV2_bp);

    // --- Torre de Foguete ---
    TowerBlueprint rocket_bp;
    rocket_bp.name = "RocketTower";
    rocket_bp.modelName = "the_rocket_tower";
    rocket_bp.cost = 175;
    rocket_bp.range = 12.0f;
    rocket_bp.cooldown = 3.0f;
    rocket_bp.targetingFactory = makeFirst;
    rocket_bp.shootingFactory = [](){ return std::unique_ptr<SplashDamageShot>(new SplashDamageShot(50.0f, 25.0f, 3.0f)); };
    rocket_bp.passiveFactory = nullptr; 
    g_shop.registerTower(rocket_bp);
    
    // --- Torre de Morteiro ---
    TowerBlueprint mortar_bp;
    mortar_bp.name = "MortarTower";
    mortar_bp.modelName = "the_mortar_tower";
    mortar_bp.cost = 250;
    mortar_bp.range = 18.0f;
    mortar_bp.cooldown = 5.0f;
    mortar_bp.targetingFactory = makeFirst;
    mortar_bp.shootingFactory = [](){ return std::unique_ptr<SplashDamageShot>(new SplashDamageShot(100.0f, 80.0f, 4.0f)); };
    mortar_bp.passiveFactory = nullptr; 
    g_shop.registerTower(mortar_bp);

    // --- Farm de Dinheiro ---
    TowerBlueprint farm_bp;
    farm_bp.name = "Farm";
    farm_bp.modelName = "the_farm";
    farm_bp.cost = 125;
    farm_bp.range = 0.0f;
    farm_bp.cooldown = 0.0f;
    farm_bp.targetingFactory = nullptr;
    farm_bp.shootingFactory = nullptr;  
    farm_bp.passiveFactory = [](){ return std::unique_ptr<GenerateIncome>(new GenerateIncome(50)); };
    g_shop.registerTower(farm_bp);

        // --- NOVA TORRE DE GELO (SUPORTE) ---
    TowerBlueprint ice_tower_bp;
    ice_tower_bp.name = "SlowTower";
    ice_tower_bp.modelName = "the_slow_tower"; 
    ice_tower_bp.cost = 50;
    ice_tower_bp.range = 7.0f;
    ice_tower_bp.cooldown = 2.5f;
    ice_tower_bp.targetingFactory = makeFirst;
    ice_tower_bp.shootingFactory = [](){ return std::unique_ptr<IceShot>(new IceShot(2.0f)); };
    ice_tower_bp.passiveFactory = nullptr;
    g_shop.registerTower(ice_tower_bp);

    printf("Setup do jogo concluído!\n");
}

// Converte a posição 2D do cursor na tela para uma posição 3D no chão do mundo
glm::vec3 GetCursorWorldPosition(GLFWwindow* window)
{
    // Obtém o tamanho da janela
    int width, height;
    glfwGetWindowSize(window, &width, &height);

    // Converte as coordenadas do cursor (pixels) para Coordenadas Normalizadas de Dispositivo (NDC) [-1, 1]
    float x_ndc = (2.0f * (float)g_cursor_x) / width - 1.0f;
    float y_ndc = 1.0f - (2.0f * (float)g_cursor_y) / height; // Y é invertido

    // Define um raio em espaço de recorte (Clip Space)
    glm::vec4 ray_clip = glm::vec4(x_ndc, y_ndc, -1.0, 1.0);

    // Converte o raio do espaço de recorte para o espaço da câmera (Eye Space)
    glm::vec4 ray_eye = inverse(g_projection_matrix) * ray_clip;
    ray_eye = glm::vec4(ray_eye.x, ray_eye.y, -1.0, 0.0); // Direção para frente

    // Converte o raio do espaço da câmera para o espaço do mundo (World Space)
    glm::vec3 ray_world_dir = glm::normalize(glm::vec3(inverse(g_view_matrix) * ray_eye));

    // Calcula a interseção do raio com o plano do chão (y = -1.0)
    // Posição da câmera (origem do raio)
    glm::vec3 ray_origin = glm::vec3(inverse(g_view_matrix)[3]);
    
    // Se o raio não estiver apontando para baixo, não haverá interseção
    if (ray_world_dir.y >= 0.0f) {
        return glm::vec3(0.0f, -999.0f, 0.0f); // Retorna uma posição inválida
    }
    
    // Calcula a distância 't' até a interseção
    float t = (-1.0f - ray_origin.y) / ray_world_dir.y;

    // Retorna o ponto de interseção
    return ray_origin + t * ray_world_dir;
}

void SellSelectedTower()
{
    if (!g_selectedTower) return;

    // Encontra e remove a torre do vetor g_towers
    g_towers.erase(std::remove_if(g_towers.begin(), g_towers.end(), 
        [](const std::unique_ptr<Tower>& tower) {
            return tower.get() == g_selectedTower;
        }), 
        g_towers.end()
    );

    // Devolve 75% do custo da torre para o jogador (exemplo)
    const auto& bp = g_shop.getBlueprint(g_selectedTower->blueprintName); // Requer um método getBlueprint na Shop
    if (bp) {
        g_playerMoney += bp->cost * 0.75f;
    }

    printf("Torre vendida!\n");
    g_selectedTower = nullptr;
    g_isTowerInfoWindowOpen = false;
}

void ToggleTowerFOV()
{
    if (g_cameraMode == CameraMode::ORBIT && g_selectedTower != nullptr) {
        g_cameraMode = CameraMode::TOWER_FOV;
        printf("Câmera em modo Terceira Pessoa.\n");
    } else {
        g_cameraMode = CameraMode::ORBIT;
        printf("Câmera em modo Orbita.\n");
    }
}

int main(int argc, char* argv[])
{
    // Inicializamos a biblioteca GLFW, utilizada para criar uma janela do
    // sistema operacional, onde poderemos renderizar com OpenGL.
    int success = glfwInit();
    if (!success)
    {
        fprintf(stderr, "ERROR: glfwInit() failed.\n");
        std::exit(EXIT_FAILURE);
    }
    // Definimos o callback para impressão de erros da GLFW no terminal
    glfwSetErrorCallback(ErrorCallback);

    // Pedimos para utilizar OpenGL versão 3.3 (ou superior)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

    #ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    #endif

    // Pedimos para utilizar o perfil "core", isto é, utilizaremos somente as
    // funções modernas de OpenGL.
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Criamos uma janela do sistema operacional, com 800 colunas e 600 linhas
    // de pixels, e com título "INF01047 ...".
    GLFWwindow* window;
    window = glfwCreateWindow(800, 600, "INF01047 - Trabalho final", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        fprintf(stderr, "ERROR: glfwCreateWindow() failed.\n");
        std::exit(EXIT_FAILURE);
    }

    // Definimos a função de callback que será chamada sempre que o usuário
    // pressionar alguma tecla do teclado ...
    glfwSetKeyCallback(window, KeyCallback);
    // ... ou clicar os botões do mouse ...
    glfwSetMouseButtonCallback(window, MouseButtonCallback);
    // ... ou movimentar o cursor do mouse em cima da janela ...
    glfwSetCursorPosCallback(window, CursorPosCallback);
    // ... ou rolar a "rodinha" do mouse.
    glfwSetScrollCallback(window, ScrollCallback);

    // Indicamos que as chamadas OpenGL deverão renderizar nesta janela
    glfwMakeContextCurrent(window);

    // Carregamento de todas funções definidas por OpenGL 3.3, utilizando a
    // biblioteca GLAD.
    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);

    // Definimos a função de callback que será chamada sempre que a janela for
    // redimensionada, por consequência alterando o tamanho do "framebuffer"
    // (região de memória onde são armazenados os pixels da imagem).
    glfwSetFramebufferSizeCallback(window, FramebufferSizeCallback);
    FramebufferSizeCallback(window, 800, 600); // Forçamos a chamada do callback acima, para definir g_ScreenRatio.

    // Imprimimos no terminal informações sobre a GPU do sistema
    const GLubyte *vendor      = glGetString(GL_VENDOR);
    const GLubyte *renderer    = glGetString(GL_RENDERER);
    const GLubyte *glversion   = glGetString(GL_VERSION);
    const GLubyte *glslversion = glGetString(GL_SHADING_LANGUAGE_VERSION);

    printf("GPU: %s, %s, OpenGL %s, GLSL %s\n", vendor, renderer, glversion, glslversion);

    // Carregamos os shaders de vértices e de fragmentos que serão utilizados
    // para renderização. Veja slides 180-200 do documento Aula_03_Rendering_Pipeline_Grafico.pdf.
    //
    LoadShadersFromFiles();

    // Carregamos duas imagens para serem utilizadas como textura
    LoadTextureImage("../../data/grama.jpg");                        // TextureImage0
    LoadTextureImage("../../data/tc-earth_nightmap_citylights.gif"); // TextureImage1
    LoadTextureImage("../../data/rocket_tower.jpg");                 // TextureImage2
    LoadTextureImage("../../data/farm.jpg");                         // TextureImage3
    LoadTextureImage("../../data/cannon_tower.jpg");                 // TextureImage4
    LoadTextureImage("../../data/mortar_tower.jpg");                 // TextureImage5
    LoadTextureImage("../../data/ceu.jpg");                          // TextureImage6
    LoadTextureImage("../../data/slow_tower.jpg");                   // TexturaImage7



    // Construímos a representação de objetos geométricos através de malhas de triângulos
    ObjModel spheremodel("../../data/sphere.obj");
    ComputeNormals(&spheremodel);
    BuildTrianglesAndAddToVirtualScene(&spheremodel);

    ObjModel bunnymodel("../../data/bunny.obj");
    ComputeNormals(&bunnymodel);
    BuildTrianglesAndAddToVirtualScene(&bunnymodel);

    ObjModel planemodel("../../data/plane.obj");
    ComputeNormals(&planemodel);
    BuildTrianglesAndAddToVirtualScene(&planemodel);

    ObjModel rocketmodel("../../data/rocket_tower.obj");
    ComputeNormals(&rocketmodel);
    BuildTrianglesAndAddToVirtualScene(&rocketmodel);

    ObjModel farmmodel("../../data/farm.obj");
    ComputeNormals(&farmmodel);
    BuildTrianglesAndAddToVirtualScene(&farmmodel);

    ObjModel cannonmodel("../../data/cannon_tower.obj");
    ComputeNormals(&cannonmodel);
    BuildTrianglesAndAddToVirtualScene(&cannonmodel);

    ObjModel mortarmodel("../../data/mortar_tower.obj");
    ComputeNormals(&mortarmodel);
    BuildTrianglesAndAddToVirtualScene(&mortarmodel);

    ObjModel slowmodel("../../data/slow_tower.obj");
    ComputeNormals(&slowmodel);
    BuildTrianglesAndAddToVirtualScene(&slowmodel);

    if ( argc > 1 )
    {
        ObjModel model(argv[1]);
        BuildTrianglesAndAddToVirtualScene(&model);
    }

    // Inicializamos o código para renderização de texto.
    TextRendering_Init();

    // Habilitamos o Z-buffer. Veja slides 104-116 do documento Aula_09_Projecoes.pdf.
    glEnable(GL_DEPTH_TEST);

    // Habilitamos o Backface Culling. Veja slides 8-13 do documento Aula_02_Fundamentos_Matematicos.pdf, slides 23-34 do documento Aula_13_Clipping_and_Culling.pdf e slides 112-123 do documento Aula_14_Laboratorio_3_Revisao.pdf.
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    //
    SetupGame();

    // Ficamos em um loop infinito, renderizando, até que o usuário feche a janela
    while (!glfwWindowShouldClose(window))
    {
        // Computamos a posição da câmera utilizando coordenadas esféricas
        float current_time = (float)glfwGetTime();
        delta_t = current_time - prev_time;
        prev_time = current_time;

        if (g_isRoundActive)
        {
            static float spawnTimer = 1.0f; // Timer para controlar a velocidade de spawn
            spawnTimer -= delta_t;
            
            if (spawnTimer <= 0.0f && g_enemiesToSpawnForRound > 0)
            {
                // Cria um novo inimigo
                auto newEnemy = std::unique_ptr<Enemy>(new Enemy(
                    g_enemyPath->getStartPoint(),
                    0.5f, 
                    100.0f + g_currentRound * 20, // Vida aumenta com os rounds
                    3.0f,
                    EnemyAttribute::RESISTANT,
                    10 + g_currentRound * 2,      // Recompensa aumenta com os rounds
                    std::unique_ptr<BezierMovement>(new BezierMovement(g_enemyPath))
                ));
                g_enemyManager.spawn(std::move(newEnemy));
                
                spawnTimer = 1.5f; // Reseta o timer para o próximo spawn
                g_enemiesToSpawnForRound--;
            }

            // Atualiza a lógica de todos os inimigos (movimento, etc.)
            g_enemyManager.updateAll(delta_t, g_playerMoney);
            auto enemy_pointers = g_enemyManager.getEnemyPointers();

            // Atualiza a lógica de todas as torres (mirar e atirar)
            for(const auto& tower : g_towers) {
                tower->update(delta_t, enemy_pointers);
            }

            if (enemy_pointers.empty() && g_enemiesToSpawnForRound == 0) {
                g_isRoundActive = false;
                g_intermissionTimer = 10.0f; // Começa a contagem para o próximo round
                printf("Round %d concluido!\n", g_currentRound);
                for(const auto& tower : g_towers) {
                    tower->updateEndOfRound(g_playerMoney);
                }
            }
        }
        else // Pausa entre os rounds
        {
            g_intermissionTimer -= delta_t;
        }

        // Aqui executamos as operações de renderização

        // Definimos a cor do "fundo" do framebuffer como branco.  Tal cor é
        // definida como coeficientes RGBA: Red, Green, Blue, Alpha; isto é:
        // Vermelho, Verde, Azul, Alpha (valor de transparência).
        // Conversaremos sobre sistemas de cores nas aulas de Modelos de Iluminação.
        //
        //           R     G     B     A
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

        // "Pintamos" todos os pixels do framebuffer com a cor definida acima,
        // e também resetamos todos os pixels do Z-buffer (depth buffer).
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Pedimos para a GPU utilizar o programa de GPU criado acima (contendo
        // os shaders de vértice e fragmentos).
        glUseProgram(g_GpuProgramID);


        glm::mat4 view;
        glm::vec4 camera_position_c;
        glm::vec4 camera_view_vector;
        glm::vec4 camera_up_vector = glm::vec4(0.0f,1.0f,0.0f,0.0f);

        if (g_cameraMode == CameraMode::TOWER_FOV && g_selectedTower != nullptr)
        {
            // MODO 1: CÂMERA EM TERCEIRA PESSOA
            glm::vec3 tower_pos = g_selectedTower->pos;
            float tower_rotation_y = g_selectedTower->currentYRotation;
            glm::vec3 offset = glm::vec3(sin(tower_rotation_y) * 4.0f, -2.0f, cos(tower_rotation_y) * 4.0f);
            camera_position_c = glm::vec4(tower_pos - offset, 1.0f);
            glm::vec4 camera_lookat_l = glm::vec4(tower_pos, 1.0f);
            camera_view_vector = camera_lookat_l - camera_position_c;
        }
        else
        {
            // MODO 2: CÂMERA ORBITAL COM MOVIMENTO WASD
            float r = g_CameraDistance;
            float y = r*sin(g_CameraPhi);
            float z = r*cos(g_CameraPhi)*cos(g_CameraTheta);
            float x = r*cos(g_CameraPhi)*sin(g_CameraTheta);
            
            camera_position_c = glm::vec4(x, y, z, 1.0f);
            glm::vec4 camera_lookat_l = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
            camera_view_vector = camera_lookat_l - camera_position_c;
            glm::vec4 camera_up_vector   = glm::vec4(0.0f,1.0f,0.0f,0.0f);

            glm::vec4 w = -camera_view_vector / norm(camera_view_vector);
            glm::vec4 u = crossproduct(camera_up_vector, w);
            
            if (press_W) g_camera_position_c -= w * delta_t * speed;
            if (press_S) g_camera_position_c += w * delta_t * speed;
            if (press_A) g_camera_position_c -= u * delta_t * speed;
            if (press_D) g_camera_position_c += u * delta_t * speed;

            camera_position_c = g_camera_position_c;
        }
        
        view = Matrix_Camera_View(camera_position_c, camera_view_vector, camera_up_vector);
        glm::mat4 projection;
        float nearplane = -0.1f;
        float farplane  = -299.0f;

        if (g_UsePerspectiveProjection)
        {
            float field_of_view = 3.141592 / 3.0f;
            projection = Matrix_Perspective(field_of_view, g_ScreenRatio, nearplane, farplane);
        }
        else
        {
            float t = 1.5f * g_CameraDistance / 2.5f;
            float b = -t;
            float r = t * g_ScreenRatio;
            float l = -r;
            projection = Matrix_Orthographic(l, r, b, t, -nearplane, -farplane); // Os planos são positivos aqui
        }
        glm::mat4 model = Matrix_Identity(); // Transformação identidade de modelagem
        g_view_matrix = view;
        g_projection_matrix = projection;

        glUseProgram(g_GpuProgramID);
        glUniformMatrix4fv(g_view_uniform, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(g_projection_uniform, 1, GL_FALSE, glm::value_ptr(projection));


        #define SPHERE 0
        #define BUNNY  1
        #define PLANE  2
        #define ROCKET 3
        #define FARM 4
        #define CANNON 5
        #define MORTAR 6
        #define SKYBOX 7
        #define RANGE_INDICATOR 8
        #define SLOW 9


        glDepthFunc(GL_LEQUAL); 
        glUseProgram(g_GpuProgramID); 
        model = Matrix_Translate(camera_position_c.x, camera_position_c.y, camera_position_c.z) 
            * Matrix_Scale(95.0f, 95.0f, 95.0f);
        glUniformMatrix4fv(g_model_uniform, 1, GL_FALSE, glm::value_ptr(model));
        glUniform1i(g_object_id_uniform, SKYBOX);
        DrawVirtualObject("the_sphere"); // Usando a esfera como skybox
        glDepthFunc(GL_LESS); 

            // Define qual ID e qual textura vamos usar para os "carimbos"
        glUniform1i(g_object_id_uniform, PLANE);

        float path_length = g_enemyPath->getTotalLength();
        float stamp_spacing = 1.5f; // Distância entre cada carimbo de textura
        float stamp_size = 2.5f;    // Tamanho de cada carimbo

        // Percorre o caminho, desenhando um carimbo a cada 'stamp_spacing' unidades
        for (float dist = 0; dist < path_length; dist += stamp_spacing)
        {
            // Pega o parâmetro 't' e a posição 3D para a distância atual
            float t = g_enemyPath->getTForDistance(dist);
            glm::vec3 position = g_enemyPath->getPoint(t);

            // Calcula a direção do caminho nesse ponto para orientar o carimbo
            glm::vec3 tangent = glm::normalize(g_enemyPath->getPoint(t + 0.01f) - position);
            float path_angle = atan2(tangent.x, tangent.z);

            // Adiciona um pouco de aleatoriedade para um visual mais natural
            float random_rotation = 6.28f; // Rotação aleatória
            float random_offset_x = 0.5f; // Pequeno desvio lateral
            float random_offset_z = 0.5f;

            position.x += random_offset_x;
            position.z += random_offset_z;

            // Cria a matriz 'model' para este carimbo específico
            model = Matrix_Translate(position.x, position.y - 0.3f, position.z)
                * Matrix_Rotate_Y(path_angle + random_rotation)
                * Matrix_Scale(stamp_size, stamp_size, stamp_size);
                
            glUniformMatrix4fv(g_model_uniform, 1, GL_FALSE, glm::value_ptr(model));
            
            // Desenha um único "pedaço de terra"
            DrawVirtualObject("the_plane");
        }
        // Desenhamos o modelo da esfera
        model = Matrix_Translate(-1.0f,0.0f,0.0f)
              * Matrix_Rotate_Z(0.6f)
              * Matrix_Rotate_X(0.2f)
              * Matrix_Rotate_Y(g_AngleY + (float)glfwGetTime() * 0.1f);
        glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
        glUniform1i(g_object_id_uniform, SPHERE);
        DrawVirtualObject("the_sphere");

        // Desenhamos o modelo do coelho
        model = Matrix_Translate(1.0f,0.0f,0.0f)
              * Matrix_Rotate_X(g_AngleX + (float)glfwGetTime() * 0.1f);
        glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
        glUniform1i(g_object_id_uniform, BUNNY);
        DrawVirtualObject("the_bunny");

        // Desenhamos o plano do chão
        // Desenha o chão
        model = Matrix_Translate(0.0f, -1.5f, 0.0f) * Matrix_Scale(25.0f, 1.0f, 25.0f);
        glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
        glUniform1i(g_object_id_uniform, PLANE);
        DrawVirtualObject("the_plane");

        // Desenhamos a Torre de foguete
        model = Matrix_Translate(2.0f,0.0f,0.0f);
        glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
        glUniform1i(g_object_id_uniform, ROCKET);
        DrawVirtualObject("the_rocket_tower");

        // Desenhamos a Torre da Fazenda
        model = Matrix_Translate(4.0f,0.0f,0.0f);
        glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
        glUniform1i(g_object_id_uniform, FARM);
        DrawVirtualObject("the_farm");

        // Desenhamos a Torre de Canhão
        model = Matrix_Translate(6.0f,0.0f,0.0f);
        glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
        glUniform1i(g_object_id_uniform, CANNON);
        DrawVirtualObject("the_cannon_tower");

        // Desenhamos a Torre de Morteiro
        model = Matrix_Translate(8.0f,0.0f,0.0f);
        glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
        glUniform1i(g_object_id_uniform, MORTAR);
        DrawVirtualObject("the_mortar_tower");

        model = Matrix_Translate(10.0f,0.0f,0.0f);
        glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
        glUniform1i(g_object_id_uniform, SLOW);
        DrawVirtualObject("the_slow_tower");

        // Loop que desenha torre ao comprar
        for (const auto& tower : g_towers)
        {
            // Pega a posição da torre
            model = Matrix_Translate(tower->pos.x, 0, tower->pos.z)
                    * Matrix_Rotate_Y(tower->currentYRotation); 
                   
            glUniformMatrix4fv(g_model_uniform, 1, GL_FALSE, glm::value_ptr(model));

            // Define o ID para o shader saber como colorir/texturizar
            if (tower->blueprintName.find("CannonTower_V1") != std::string::npos) {
                glUniform1i(g_object_id_uniform, CANNON);
            } else if (tower->blueprintName == "Farm") {
                glUniform1i(g_object_id_uniform, FARM);
            } else if (tower->blueprintName == "RocketTower") {
                glUniform1i(g_object_id_uniform, ROCKET);
            } else if (tower->blueprintName == "MortarTower") {
                glUniform1i(g_object_id_uniform, MORTAR);
            } else if (tower->blueprintName == "SlowTower") {
                glUniform1i(g_object_id_uniform, SLOW);
            } 

            // Desenha o modelo
            DrawVirtualObject(tower->modelName.c_str());
        }

        auto enemy_pointers = g_enemyManager.getEnemyPointers();


        // Percorre a lista de inimigos e desenha cada um
        for (const auto* enemy : enemy_pointers)
        {
            //  Cria a matriz para posicionar o inimigo no mundo
            model = Matrix_Translate(enemy->hitbox.center.x, enemy->hitbox.center.y, enemy->hitbox.center.z)
                * Matrix_Scale(enemy->hitbox.radius, enemy->hitbox.radius, enemy->hitbox.radius); 
            glUniformMatrix4fv(g_model_uniform, 1, GL_FALSE, glm::value_ptr(model));

            //   desenha o modelo do inimigo (Usando o coelho pra testar)
            DrawVirtualObject("the_bunny");
        }

        // desenha o range da torre
        if (g_selectedTower != nullptr)
        {
            // Habilita a transparência
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            
            // Posição da esfera é o centro da torre selecionada
            model = Matrix_Translate(g_selectedTower->pos.x, g_selectedTower->pos.y, g_selectedTower->pos.z)
                    * Matrix_Scale(g_selectedTower->range, g_selectedTower->range, g_selectedTower->range);
            
            glUniformMatrix4fv(g_model_uniform, 1, GL_FALSE, glm::value_ptr(model));
            glUniform1i(g_object_id_uniform, RANGE_INDICATOR);
            
            // CORREÇÃO: Desenha a geometria da esfera em vez do plano
            DrawVirtualObject("the_sphere"); 
            
            // Desabilita a transparência para não afetar outros objetos
            glDisable(GL_BLEND);
        }


        // HUD
        // FIM DO HUD
        char buffer[100];
        snprintf(buffer, sizeof(buffer), "Dinheiro: %.0f", g_playerMoney);
        TextRendering_PrintString(window, buffer, -0.95f, 0.95f);
        snprintf(buffer, sizeof(buffer), "Round: %d", g_currentRound);
        TextRendering_PrintString(window, buffer, -0.95f, 0.90f);

        if (!g_isRoundActive && g_currentRound < g_totalRounds) {
            snprintf(buffer, sizeof(buffer), "Pressione 'G' para iniciar o Round %d", g_currentRound + 1);
            TextRendering_PrintString(window, buffer, -0.4f, 0.0f);
        }

        if (g_selectedTower)
        {
            float text_y = -0.40f;
            float line_height = 0.05f;

           // Mostra o nome da torre
            snprintf(buffer, sizeof(buffer), "Torre Selecionada: %s", g_selectedTower->blueprintName.c_str());
            TextRendering_PrintString(window, buffer, -0.95f, text_y);

            // Verifica se a torre pode atacar para mostrar Dano, Cooldown e Foco
            if (g_selectedTower->shooting) 
            {
                
                // Mostra o Range
                snprintf(buffer, sizeof(buffer), "- Range: %.1f", g_selectedTower->range);
                TextRendering_PrintString(window, buffer, -0.95f, text_y - line_height);
            
                // Mostra o Cooldown
                snprintf(buffer, sizeof(buffer), "- Cooldown: %.1f", g_selectedTower->cooldown);
                TextRendering_PrintString(window, buffer, -0.95f, text_y - 2*line_height);

                // Mostra o Dano
                std::string damage_info = g_selectedTower->shooting->getDamageInfo();
                snprintf(buffer, sizeof(buffer), "- Dano: %s", damage_info.c_str());
                TextRendering_PrintString(window, buffer, -0.95f, text_y - 3*line_height);

                // MOSTRA O FOCO ATUAL DA TORRE
                if (g_selectedTower->targeting) {
                    std::string mode_info = g_selectedTower->targeting->getModeName();
                    snprintf(buffer, sizeof(buffer), "- Foco: %s", mode_info.c_str());
                    TextRendering_PrintString(window, buffer, -0.95f, text_y - 4*line_height);
                }
            }
            // Se for uma torre passiva (Farm), mostra sua habilidade
            else if (g_selectedTower->passiveAbility)
            {
                std::string passive_info = g_selectedTower->passiveAbility->getInfo();
                snprintf(buffer, sizeof(buffer), "- Habilidade: %s", passive_info.c_str());
                TextRendering_PrintString(window, buffer, -0.95f, text_y - 2*line_height);
            }
            

            // Mostra as opções (botões)
            TextRendering_PrintString(window, "[U] - Upgrade", -0.95f, text_y - 5*line_height);
            TextRendering_PrintString(window, "[V] - Vender", -0.95f, text_y - 6*line_height);
            TextRendering_PrintString(window, "[F] - Ver da Torre", -0.95f, text_y - 7*line_height);
            TextRendering_PrintString(window, "[Q] - Mudar Foco", -0.95f, text_y - 8*line_height);
        }



        TextRendering_ShowEulerAngles(window);

        // Imprimimos na informação sobre a matriz de projeção sendo utilizada.
        TextRendering_ShowProjection(window);

        // Imprimimos na tela informação sobre o número de quadros renderizados
        // por segundo (frames per second).
        TextRendering_ShowFramesPerSecond(window);

        // O framebuffer onde OpenGL executa as operações de renderização não
        // é o mesmo que está sendo mostrado para o usuário, caso contrário
        // seria possível ver artefatos conhecidos como "screen tearing". A
        // chamada abaixo faz a troca dos buffers, mostrando para o usuário
        // tudo que foi renderizado pelas funções acima.
        // Veja o link: https://en.wikipedia.org/w/index.php?title=Multiple_buffering&oldid=793452829#Double_buffering_in_computer_graphics
        glfwSwapBuffers(window);

        // Verificamos com o sistema operacional se houve alguma interação do
        // usuário (teclado, mouse, ...). Caso positivo, as funções de callback
        // definidas anteriormente usando glfwSet*Callback() serão chamadas
        // pela biblioteca GLFW.
        glfwPollEvents();
    }
    // Finalizamos o uso dos recursos do sistema operacional
    glfwTerminate();

    // Fim do programa
    return 0;
}

// Função que carrega uma imagem para ser utilizada como textura
void LoadTextureImage(const char* filename)
{
    printf("Carregando imagem \"%s\"... ", filename);

    // Primeiro fazemos a leitura da imagem do disco
    stbi_set_flip_vertically_on_load(true);
    int width;
    int height;
    int channels;
    unsigned char *data = stbi_load(filename, &width, &height, &channels, 3);

    if ( data == NULL )
    {
        fprintf(stderr, "ERROR: Cannot open image file \"%s\".\n", filename);
        std::exit(EXIT_FAILURE);
    }

    printf("OK (%dx%d).\n", width, height);

    // Agora criamos objetos na GPU com OpenGL para armazenar a textura
    GLuint texture_id;
    GLuint sampler_id;
    glGenTextures(1, &texture_id);
    glGenSamplers(1, &sampler_id);

    // Veja slides 95-96 do documento Aula_20_Mapeamento_de_Texturas.pdf
    glSamplerParameteri(sampler_id, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glSamplerParameteri(sampler_id, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Parâmetros de amostragem da textura.
    glSamplerParameteri(sampler_id, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glSamplerParameteri(sampler_id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Agora enviamos a imagem lida do disco para a GPU
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
    glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);

    GLuint textureunit = g_NumLoadedTextures;
    glActiveTexture(GL_TEXTURE0 + textureunit);
    glBindTexture(GL_TEXTURE_2D, texture_id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindSampler(textureunit, sampler_id);

    stbi_image_free(data);

    g_NumLoadedTextures += 1;
}

// Função que desenha um objeto armazenado em g_VirtualScene. Veja definição
// dos objetos na função BuildTrianglesAndAddToVirtualScene().
void DrawVirtualObject(const char* object_name)
{
    // "Ligamos" o VAO. Informamos que queremos utilizar os atributos de
    // vértices apontados pelo VAO criado pela função BuildTrianglesAndAddToVirtualScene(). Veja
    // comentários detalhados dentro da definição de BuildTrianglesAndAddToVirtualScene().
    glBindVertexArray(g_VirtualScene[object_name].vertex_array_object_id);

    // Setamos as variáveis "bbox_min" e "bbox_max" do fragment shader
    // com os parâmetros da axis-aligned bounding box (AABB) do modelo.
    glm::vec3 bbox_min = g_VirtualScene[object_name].bbox_min;
    glm::vec3 bbox_max = g_VirtualScene[object_name].bbox_max;
    glUniform4f(g_bbox_min_uniform, bbox_min.x, bbox_min.y, bbox_min.z, 1.0f);
    glUniform4f(g_bbox_max_uniform, bbox_max.x, bbox_max.y, bbox_max.z, 1.0f);

    // Pedimos para a GPU rasterizar os vértices dos eixos XYZ
    // apontados pelo VAO como linhas. Veja a definição de
    // g_VirtualScene[""] dentro da função BuildTrianglesAndAddToVirtualScene(), e veja
    // a documentação da função glDrawElements() em
    // http://docs.gl/gl3/glDrawElements.
    glDrawElements(
        g_VirtualScene[object_name].rendering_mode,
        g_VirtualScene[object_name].num_indices,
        GL_UNSIGNED_INT,
        (void*)(g_VirtualScene[object_name].first_index * sizeof(GLuint))
    );

    // "Desligamos" o VAO, evitando assim que operações posteriores venham a
    // alterar o mesmo. Isso evita bugs.
    glBindVertexArray(0);
}


// Função que carrega os shaders de vértices e de fragmentos que serão
// utilizados para renderização. Veja slides 180-200 do documento Aula_03_Rendering_Pipeline_Grafico.pdf.
//
void LoadShadersFromFiles()
{
    // Note que o caminho para os arquivos "shader_vertex.glsl" e
    // "shader_fragment.glsl" estão fixados, sendo que assumimos a existência
    // da seguinte estrutura no sistema de arquivos:
    //
    //    + FCG_Lab_01/
    //    |
    //    +--+ bin/
    //    |  |
    //    |  +--+ Release/  (ou Debug/ ou Linux/)
    //    |     |
    //    |     o-- main.exe
    //    |
    //    +--+ src/
    //       |
    //       o-- shader_vertex.glsl
    //       |
    //       o-- shader_fragment.glsl
    //
    GLuint vertex_shader_id = LoadShader_Vertex("../../src/shader_vertex.glsl");
    GLuint fragment_shader_id = LoadShader_Fragment("../../src/shader_fragment.glsl");

    // Deletamos o programa de GPU anterior, caso ele exista.
    if ( g_GpuProgramID != 0 )
        glDeleteProgram(g_GpuProgramID);

    // Criamos um programa de GPU utilizando os shaders carregados acima.
    g_GpuProgramID = CreateGpuProgram(vertex_shader_id, fragment_shader_id);

    // Buscamos o endereço das variáveis definidas dentro do Vertex Shader.
    // Utilizaremos estas variáveis para enviar dados para a placa de vídeo
    // (GPU)! Veja arquivo "shader_vertex.glsl" e "shader_fragment.glsl".
    g_model_uniform      = glGetUniformLocation(g_GpuProgramID, "model"); // Variável da matriz "model"
    g_view_uniform       = glGetUniformLocation(g_GpuProgramID, "view"); // Variável da matriz "view" em shader_vertex.glsl
    g_projection_uniform = glGetUniformLocation(g_GpuProgramID, "projection"); // Variável da matriz "projection" em shader_vertex.glsl
    g_object_id_uniform  = glGetUniformLocation(g_GpuProgramID, "object_id"); // Variável "object_id" em shader_fragment.glsl
    g_bbox_min_uniform   = glGetUniformLocation(g_GpuProgramID, "bbox_min");
    g_bbox_max_uniform   = glGetUniformLocation(g_GpuProgramID, "bbox_max");

    // Variáveis em "shader_fragment.glsl" para acesso das imagens de textura
    glUseProgram(g_GpuProgramID);
    glUniform1i(glGetUniformLocation(g_GpuProgramID, "TextureImage0"), 0);
    glUniform1i(glGetUniformLocation(g_GpuProgramID, "TextureImage1"), 1);
    glUniform1i(glGetUniformLocation(g_GpuProgramID, "TextureImage2"), 2);
    glUniform1i(glGetUniformLocation(g_GpuProgramID, "TextureImage3"), 3);
    glUniform1i(glGetUniformLocation(g_GpuProgramID, "TextureImage4"), 4);
    glUniform1i(glGetUniformLocation(g_GpuProgramID, "TextureImage5"), 5);
    glUniform1i(glGetUniformLocation(g_GpuProgramID, "TextureImage6"), 6);
    glUniform1i(glGetUniformLocation(g_GpuProgramID, "TextureImage7"), 7);
    glUseProgram(0);
}

// Função que pega a matriz M e guarda a mesma no topo da pilha
void PushMatrix(glm::mat4 M)
{
    g_MatrixStack.push(M);
}

// Função que remove a matriz atualmente no topo da pilha e armazena a mesma na variável M
void PopMatrix(glm::mat4& M)
{
    if ( g_MatrixStack.empty() )
    {
        M = Matrix_Identity();
    }
    else
    {
        M = g_MatrixStack.top();
        g_MatrixStack.pop();
    }
}

// Função que computa as normais de um ObjModel, caso elas não tenham sido
// especificadas dentro do arquivo ".obj"
void ComputeNormals(ObjModel* model)
{
    if ( !model->attrib.normals.empty() )
        return;

    // Primeiro computamos as normais para todos os TRIÂNGULOS.
    // Segundo, computamos as normais dos VÉRTICES através do método proposto
    // por Gouraud, onde a normal de cada vértice vai ser a média das normais de
    // todas as faces que compartilham este vértice.

    size_t num_vertices = model->attrib.vertices.size() / 3;

    std::vector<int> num_triangles_per_vertex(num_vertices, 0);
    std::vector<glm::vec4> vertex_normals(num_vertices, glm::vec4(0.0f,0.0f,0.0f,0.0f));

    for (size_t shape = 0; shape < model->shapes.size(); ++shape)
    {
        size_t num_triangles = model->shapes[shape].mesh.num_face_vertices.size();

        for (size_t triangle = 0; triangle < num_triangles; ++triangle)
        {
            assert(model->shapes[shape].mesh.num_face_vertices[triangle] == 3);

            glm::vec4  vertices[3];
            for (size_t vertex = 0; vertex < 3; ++vertex)
            {
                tinyobj::index_t idx = model->shapes[shape].mesh.indices[3*triangle + vertex];
                const float vx = model->attrib.vertices[3*idx.vertex_index + 0];
                const float vy = model->attrib.vertices[3*idx.vertex_index + 1];
                const float vz = model->attrib.vertices[3*idx.vertex_index + 2];
                vertices[vertex] = glm::vec4(vx,vy,vz,1.0);
            }

            const glm::vec4  a = vertices[0];
            const glm::vec4  b = vertices[1];
            const glm::vec4  c = vertices[2];

            const glm::vec4  n = crossproduct(b-a,c-a);

            for (size_t vertex = 0; vertex < 3; ++vertex)
            {
                tinyobj::index_t idx = model->shapes[shape].mesh.indices[3*triangle + vertex];
                num_triangles_per_vertex[idx.vertex_index] += 1;
                vertex_normals[idx.vertex_index] += n;
                model->shapes[shape].mesh.indices[3*triangle + vertex].normal_index = idx.vertex_index;
            }
        }
    }

    model->attrib.normals.resize( 3*num_vertices );

    for (size_t i = 0; i < vertex_normals.size(); ++i)
    {
        glm::vec4 n = vertex_normals[i] / (float)num_triangles_per_vertex[i];
        n /= norm(n);
        model->attrib.normals[3*i + 0] = n.x;
        model->attrib.normals[3*i + 1] = n.y;
        model->attrib.normals[3*i + 2] = n.z;
    }
}

// Constrói triângulos para futura renderização a partir de um ObjModel.
void BuildTrianglesAndAddToVirtualScene(ObjModel* model)
{
    GLuint vertex_array_object_id;
    glGenVertexArrays(1, &vertex_array_object_id);
    glBindVertexArray(vertex_array_object_id);

    std::vector<GLuint> indices;
    std::vector<float>  model_coefficients;
    std::vector<float>  normal_coefficients;
    std::vector<float>  texture_coefficients;

    for (size_t shape = 0; shape < model->shapes.size(); ++shape)
    {
        size_t first_index = indices.size();
        size_t num_triangles = model->shapes[shape].mesh.num_face_vertices.size();

        const float minval = std::numeric_limits<float>::min();
        const float maxval = std::numeric_limits<float>::max();

        glm::vec3 bbox_min = glm::vec3(maxval,maxval,maxval);
        glm::vec3 bbox_max = glm::vec3(minval,minval,minval);

        for (size_t triangle = 0; triangle < num_triangles; ++triangle)
        {
            assert(model->shapes[shape].mesh.num_face_vertices[triangle] == 3);

            for (size_t vertex = 0; vertex < 3; ++vertex)
            {
                tinyobj::index_t idx = model->shapes[shape].mesh.indices[3*triangle + vertex];

                indices.push_back(first_index + 3*triangle + vertex);

                const float vx = model->attrib.vertices[3*idx.vertex_index + 0];
                const float vy = model->attrib.vertices[3*idx.vertex_index + 1];
                const float vz = model->attrib.vertices[3*idx.vertex_index + 2];
                //printf("tri %d vert %d = (%.2f, %.2f, %.2f)\n", (int)triangle, (int)vertex, vx, vy, vz);
                model_coefficients.push_back( vx ); // X
                model_coefficients.push_back( vy ); // Y
                model_coefficients.push_back( vz ); // Z
                model_coefficients.push_back( 1.0f ); // W

                bbox_min.x = std::min(bbox_min.x, vx);
                bbox_min.y = std::min(bbox_min.y, vy);
                bbox_min.z = std::min(bbox_min.z, vz);
                bbox_max.x = std::max(bbox_max.x, vx);
                bbox_max.y = std::max(bbox_max.y, vy);
                bbox_max.z = std::max(bbox_max.z, vz);

                // Inspecionando o código da tinyobjloader, o aluno Bernardo
                // Sulzbach (2017/1) apontou que a maneira correta de testar se
                // existem normais e coordenadas de textura no ObjModel é
                // comparando se o índice retornado é -1. Fazemos isso abaixo.

                if ( idx.normal_index != -1 )
                {
                    const float nx = model->attrib.normals[3*idx.normal_index + 0];
                    const float ny = model->attrib.normals[3*idx.normal_index + 1];
                    const float nz = model->attrib.normals[3*idx.normal_index + 2];
                    normal_coefficients.push_back( nx ); // X
                    normal_coefficients.push_back( ny ); // Y
                    normal_coefficients.push_back( nz ); // Z
                    normal_coefficients.push_back( 0.0f ); // W
                }

                if ( idx.texcoord_index != -1 )
                {
                    const float u = model->attrib.texcoords[2*idx.texcoord_index + 0];
                    const float v = model->attrib.texcoords[2*idx.texcoord_index + 1];
                    texture_coefficients.push_back( u );
                    texture_coefficients.push_back( v );
                }
            }
        }

        size_t last_index = indices.size() - 1;

        SceneObject theobject;
        theobject.name           = model->shapes[shape].name;
        theobject.first_index    = first_index; // Primeiro índice
        theobject.num_indices    = last_index - first_index + 1; // Número de indices
        theobject.rendering_mode = GL_TRIANGLES;       // Índices correspondem ao tipo de rasterização GL_TRIANGLES.
        theobject.vertex_array_object_id = vertex_array_object_id;

        theobject.bbox_min = bbox_min;
        theobject.bbox_max = bbox_max;

        g_VirtualScene[model->shapes[shape].name] = theobject;
    }

    GLuint VBO_model_coefficients_id;
    glGenBuffers(1, &VBO_model_coefficients_id);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_model_coefficients_id);
    glBufferData(GL_ARRAY_BUFFER, model_coefficients.size() * sizeof(float), NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, model_coefficients.size() * sizeof(float), model_coefficients.data());
    GLuint location = 0; // "(location = 0)" em "shader_vertex.glsl"
    GLint  number_of_dimensions = 4; // vec4 em "shader_vertex.glsl"
    glVertexAttribPointer(location, number_of_dimensions, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(location);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    if ( !normal_coefficients.empty() )
    {
        GLuint VBO_normal_coefficients_id;
        glGenBuffers(1, &VBO_normal_coefficients_id);
        glBindBuffer(GL_ARRAY_BUFFER, VBO_normal_coefficients_id);
        glBufferData(GL_ARRAY_BUFFER, normal_coefficients.size() * sizeof(float), NULL, GL_STATIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0, normal_coefficients.size() * sizeof(float), normal_coefficients.data());
        location = 1; // "(location = 1)" em "shader_vertex.glsl"
        number_of_dimensions = 4; // vec4 em "shader_vertex.glsl"
        glVertexAttribPointer(location, number_of_dimensions, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(location);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    if ( !texture_coefficients.empty() )
    {
        GLuint VBO_texture_coefficients_id;
        glGenBuffers(1, &VBO_texture_coefficients_id);
        glBindBuffer(GL_ARRAY_BUFFER, VBO_texture_coefficients_id);
        glBufferData(GL_ARRAY_BUFFER, texture_coefficients.size() * sizeof(float), NULL, GL_STATIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0, texture_coefficients.size() * sizeof(float), texture_coefficients.data());
        location = 2; // "(location = 1)" em "shader_vertex.glsl"
        number_of_dimensions = 2; // vec2 em "shader_vertex.glsl"
        glVertexAttribPointer(location, number_of_dimensions, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(location);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    GLuint indices_id;
    glGenBuffers(1, &indices_id);

    // "Ligamos" o buffer. Note que o tipo agora é GL_ELEMENT_ARRAY_BUFFER.
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indices_id);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, indices.size() * sizeof(GLuint), indices.data());
    // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); // XXX Errado!
    //

    // "Desligamos" o VAO, evitando assim que operações posteriores venham a
    // alterar o mesmo. Isso evita bugs.
    glBindVertexArray(0);
}

// Carrega um Vertex Shader de um arquivo GLSL. Veja definição de LoadShader() abaixo.
GLuint LoadShader_Vertex(const char* filename)
{
    // Criamos um identificador (ID) para este shader, informando que o mesmo
    // será aplicado nos vértices.
    GLuint vertex_shader_id = glCreateShader(GL_VERTEX_SHADER);

    // Carregamos e compilamos o shader
    LoadShader(filename, vertex_shader_id);

    // Retorna o ID gerado acima
    return vertex_shader_id;
}

// Carrega um Fragment Shader de um arquivo GLSL . Veja definição de LoadShader() abaixo.
GLuint LoadShader_Fragment(const char* filename)
{
    // Criamos um identificador (ID) para este shader, informando que o mesmo
    // será aplicado nos fragmentos.
    GLuint fragment_shader_id = glCreateShader(GL_FRAGMENT_SHADER);

    // Carregamos e compilamos o shader
    LoadShader(filename, fragment_shader_id);

    // Retorna o ID gerado acima
    return fragment_shader_id;
}

// Função auxilar, utilizada pelas duas funções acima. Carrega código de GPU de
// um arquivo GLSL e faz sua compilação.
void LoadShader(const char* filename, GLuint shader_id)
{
    // Lemos o arquivo de texto indicado pela variável "filename"
    // e colocamos seu conteúdo em memória, apontado pela variável
    // "shader_string".
    std::ifstream file;
    try {
        file.exceptions(std::ifstream::failbit);
        file.open(filename);
    } catch ( std::exception& e ) {
        fprintf(stderr, "ERROR: Cannot open file \"%s\".\n", filename);
        std::exit(EXIT_FAILURE);
    }
    std::stringstream shader;
    shader << file.rdbuf();
    std::string str = shader.str();
    const GLchar* shader_string = str.c_str();
    const GLint   shader_string_length = static_cast<GLint>( str.length() );

    // Define o código do shader GLSL, contido na string "shader_string"
    glShaderSource(shader_id, 1, &shader_string, &shader_string_length);

    // Compila o código do shader GLSL (em tempo de execução)
    glCompileShader(shader_id);

    // Verificamos se ocorreu algum erro ou "warning" durante a compilação
    GLint compiled_ok;
    glGetShaderiv(shader_id, GL_COMPILE_STATUS, &compiled_ok);

    GLint log_length = 0;
    glGetShaderiv(shader_id, GL_INFO_LOG_LENGTH, &log_length);

    // Alocamos memória para guardar o log de compilação.
    // A chamada "new" em C++ é equivalente ao "malloc()" do C.
    GLchar* log = new GLchar[log_length];
    glGetShaderInfoLog(shader_id, log_length, &log_length, log);

    // Imprime no terminal qualquer erro ou "warning" de compilação
    if ( log_length != 0 )
    {
        std::string  output;

        if ( !compiled_ok )
        {
            output += "ERROR: OpenGL compilation of \"";
            output += filename;
            output += "\" failed.\n";
            output += "== Start of compilation log\n";
            output += log;
            output += "== End of compilation log\n";
        }
        else
        {
            output += "WARNING: OpenGL compilation of \"";
            output += filename;
            output += "\".\n";
            output += "== Start of compilation log\n";
            output += log;
            output += "== End of compilation log\n";
        }

        fprintf(stderr, "%s", output.c_str());
    }

    // A chamada "delete" em C++ é equivalente ao "free()" do C
    delete [] log;
}

// Esta função cria um programa de GPU, o qual contém obrigatoriamente um
// Vertex Shader e um Fragment Shader.
GLuint CreateGpuProgram(GLuint vertex_shader_id, GLuint fragment_shader_id)
{
    // Criamos um identificador (ID) para este programa de GPU
    GLuint program_id = glCreateProgram();

    // Definição dos dois shaders GLSL que devem ser executados pelo programa
    glAttachShader(program_id, vertex_shader_id);
    glAttachShader(program_id, fragment_shader_id);

    // Linkagem dos shaders acima ao programa
    glLinkProgram(program_id);

    // Verificamos se ocorreu algum erro durante a linkagem
    GLint linked_ok = GL_FALSE;
    glGetProgramiv(program_id, GL_LINK_STATUS, &linked_ok);

    // Imprime no terminal qualquer erro de linkagem
    if ( linked_ok == GL_FALSE )
    {
        GLint log_length = 0;
        glGetProgramiv(program_id, GL_INFO_LOG_LENGTH, &log_length);

        // Alocamos memória para guardar o log de compilação.
        // A chamada "new" em C++ é equivalente ao "malloc()" do C.
        GLchar* log = new GLchar[log_length];

        glGetProgramInfoLog(program_id, log_length, &log_length, log);

        std::string output;

        output += "ERROR: OpenGL linking of program failed.\n";
        output += "== Start of link log\n";
        output += log;
        output += "\n== End of link log\n";

        // A chamada "delete" em C++ é equivalente ao "free()" do C
        delete [] log;

        fprintf(stderr, "%s", output.c_str());
    }

    // Os "Shader Objects" podem ser marcados para deleção após serem linkados 
    glDeleteShader(vertex_shader_id);
    glDeleteShader(fragment_shader_id);

    // Retornamos o ID gerado acima
    return program_id;
}

// Definição da função que será chamada sempre que a janela do sistema
// operacional for redimensionada, por consequência alterando o tamanho do
// "framebuffer" (região de memória onde são armazenados os pixels da imagem).
void FramebufferSizeCallback(GLFWwindow* window, int width, int height)
{
    // Indicamos que queremos renderizar em toda região do framebuffer. A
    // função "glViewport" define o mapeamento das "normalized device
    // coordinates" (NDC) para "pixel coordinates".  Essa é a operação de
    // "Screen Mapping" ou "Viewport Mapping" vista em aula ({+ViewportMapping2+}).
    glViewport(0, 0, width, height);

    // Atualizamos também a razão que define a proporção da janela (largura /
    // altura), a qual será utilizada na definição das matrizes de projeção,
    // tal que não ocorra distorções durante o processo de "Screen Mapping"
    // acima, quando NDC é mapeado para coordenadas de pixels. Veja slides 205-215 do documento Aula_09_Projecoes.pdf.
    //
    // O cast para float é necessário pois números inteiros são arredondados ao
    // serem divididos!
    g_ScreenRatio = (float)width / height;
}

// Variáveis globais que armazenam a última posição do cursor do mouse, para
// que possamos calcular quanto que o mouse se movimentou entre dois instantes
// de tempo. Utilizadas no callback CursorPosCallback() abaixo.
double g_LastCursorPosX, g_LastCursorPosY;

// Função callback chamada sempre que o usuário aperta algum dos botões do mouse
void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
        // Se o usuário pressionou o botão esquerdo do mouse, guardamos a
        // posição atual do cursor nas variáveis g_LastCursorPosX e
        // g_LastCursorPosY.  Também, setamos a variável
        // g_LeftMouseButtonPressed como true, para saber que o usuário está
        // com o botão esquerdo pressionado.
        glfwGetCursorPos(window, &g_LastCursorPosX, &g_LastCursorPosY);
        g_LeftMouseButtonPressed = true;
        glm::vec3 ray_origin = glm::vec3(inverse(g_view_matrix)[3]);
        
        int width, height;
        glfwGetWindowSize(window, &width, &height);
        float x_ndc = (2.0f * (float)g_cursor_x) / width - 1.0f;
        float y_ndc = 1.0f - (2.0f * (float)g_cursor_y) / height;
        
        glm::vec4 ray_clip = glm::vec4(x_ndc, y_ndc, -1.0, 1.0);
        glm::vec4 ray_eye = inverse(g_projection_matrix) * ray_clip;
        ray_eye = glm::vec4(ray_eye.x, ray_eye.y, -1.0, 0.0);
        
        glm::vec3 ray_world_dir = glm::normalize(glm::vec3(inverse(g_view_matrix) * ray_eye));

        // 2. Itera por todas as torres para ver se o raio colide com alguma
        Tower* clicked_tower = nullptr;
        float closest_intersection = std::numeric_limits<float>::infinity();

        for (const auto& tower : g_towers)
        {
            // Teste de interseção Raio-Esfera (hitbox da torre)
            float radius = 1.0f; // Raio da hitbox da torre (pode ser definido no blueprint)
            glm::vec3 oc = ray_origin - tower->pos;
            float a = dot(ray_world_dir, ray_world_dir);
            float b = 2.0 * dot(oc, ray_world_dir);
            float c = dot(oc, oc) - radius*radius;
            float discriminant = b*b - 4*a*c;

            if (discriminant >= 0) {
                float t = (-b - sqrt(discriminant)) / (2.0f * a);
                if (t < closest_intersection) {
                    closest_intersection = t;
                    clicked_tower = tower.get();
                }
            }
        }
        
        // 3. Atualiza a torre selecionada
        g_selectedTower = clicked_tower;

        if (g_selectedTower) {
            printf("Torre '%s' selecionada!\n", g_selectedTower->blueprintName.c_str());
        } else {
            printf("Nenhuma torre selecionada.\n");
        }
    }
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
    {
        // Quando o usuário soltar o botão esquerdo do mouse, atualizamos a
        // variável abaixo para false.
        g_LeftMouseButtonPressed = false;
    }
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
    {
        // Se o usuário pressionou o botão esquerdo do mouse, guardamos a
        // posição atual do cursor nas variáveis g_LastCursorPosX e
        // g_LastCursorPosY.  Também, setamos a variável
        // g_RightMouseButtonPressed como true, para saber que o usuário está
        // com o botão esquerdo pressionado.
        glfwGetCursorPos(window, &g_LastCursorPosX, &g_LastCursorPosY);
        g_RightMouseButtonPressed = true;
    }
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE)
    {
        // Quando o usuário soltar o botão esquerdo do mouse, atualizamos a
        // variável abaixo para false.
        g_RightMouseButtonPressed = false;
    }
    if (button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_PRESS)
    {
        // Se o usuário pressionou o botão esquerdo do mouse, guardamos a
        // posição atual do cursor nas variáveis g_LastCursorPosX e
        // g_LastCursorPosY.  Também, setamos a variável
        // g_MiddleMouseButtonPressed como true, para saber que o usuário está
        // com o botão esquerdo pressionado.
        glfwGetCursorPos(window, &g_LastCursorPosX, &g_LastCursorPosY);
        g_MiddleMouseButtonPressed = true;
    }
    if (button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_RELEASE)
    {
        // Quando o usuário soltar o botão esquerdo do mouse, atualizamos a
        // variável abaixo para false.
        g_MiddleMouseButtonPressed = false;
    }
}

// Função callback chamada sempre que o usuário movimentar o cursor do mouse em
// cima da janela OpenGL.
void CursorPosCallback(GLFWwindow* window, double xpos, double ypos)
{
    // Abaixo executamos o seguinte: caso o botão esquerdo do mouse esteja
    // pressionado, computamos quanto que o mouse se movimento desde o último
    // instante de tempo, e usamos esta movimentação para atualizar os
    // parâmetros que definem a posição da câmera dentro da cena virtual.
    // Assim, temos que o usuário consegue controlar a câmera.
    g_cursor_x = xpos;
    g_cursor_y = ypos;

    if (g_LeftMouseButtonPressed)
    {
        // Deslocamento do cursor do mouse em x e y de coordenadas de tela!
        float dx = xpos - g_LastCursorPosX;
        float dy = ypos - g_LastCursorPosY;
    
        // Atualizamos parâmetros da câmera com os deslocamentos
        g_CameraTheta -= 0.01f*dx;
        g_CameraPhi   += 0.01f*dy;
    
        // Em coordenadas esféricas, o ângulo phi deve ficar entre -pi/2 e +pi/2.
        float phimax = 3.141592f/2;
        float phimin = -phimax;
    
        if (g_CameraPhi > phimax)
            g_CameraPhi = phimax;
    
        if (g_CameraPhi < phimin)
            g_CameraPhi = phimin;
    
        // Atualizamos as variáveis globais para armazenar a posição atual do
        // cursor como sendo a última posição conhecida do cursor.
        g_LastCursorPosX = xpos;
        g_LastCursorPosY = ypos;
    }

    if (g_RightMouseButtonPressed)
    {
        // Deslocamento do cursor do mouse em x e y de coordenadas de tela!
        float dx = xpos - g_LastCursorPosX;
        float dy = ypos - g_LastCursorPosY;
    
        // Atualizamos parâmetros da antebraço com os deslocamentos
        g_ForearmAngleZ -= 0.01f*dx;
        g_ForearmAngleX += 0.01f*dy;
    
        // Atualizamos as variáveis globais para armazenar a posição atual do
        // cursor como sendo a última posição conhecida do cursor.
        g_LastCursorPosX = xpos;
        g_LastCursorPosY = ypos;
    }

    if (g_MiddleMouseButtonPressed)
    {
        // Deslocamento do cursor do mouse em x e y de coordenadas de tela!
        float dx = xpos - g_LastCursorPosX;
        float dy = ypos - g_LastCursorPosY;
    
        // Atualizamos parâmetros da antebraço com os deslocamentos
        g_TorsoPositionX += 0.01f*dx;
        g_TorsoPositionY -= 0.01f*dy;
    
        // Atualizamos as variáveis globais para armazenar a posição atual do
        // cursor como sendo a última posição conhecida do cursor.
        g_LastCursorPosX = xpos;
        g_LastCursorPosY = ypos;
    }
}

// Função callback chamada sempre que o usuário movimenta a "rodinha" do mouse.
void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    // Atualizamos a distância da câmera para a origem utilizando a
    // movimentação da "rodinha", simulando um ZOOM.
    g_CameraDistance -= 0.1f*yoffset;

    // Uma câmera look-at nunca pode estar exatamente "em cima" do ponto para
    // onde ela está olhando, pois isto gera problemas de divisão por zero na
    // definição do sistema de coordenadas da câmera. Isto é, a variável abaixo
    // nunca pode ser zero. Versões anteriores deste código possuíam este bug,
    // o qual foi detectado pelo aluno Vinicius Fraga (2017/2).
    const float verysmallnumber = std::numeric_limits<float>::epsilon();
    if (g_CameraDistance < verysmallnumber)
        g_CameraDistance = verysmallnumber;
}

// Definição da função que será chamada sempre que o usuário pressionar alguma
// tecla do teclado. Veja http://www.glfw.org/docs/latest/input_guide.html#input_key
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mod)
{
    // ======================
    // Não modifique este loop! Ele é utilizando para correção automatizada dos
    // laboratórios. Deve ser sempre o primeiro comando desta função KeyCallback().
    for (int i = 0; i < 10; ++i)
        if (key == GLFW_KEY_0 + i && action == GLFW_PRESS && mod == GLFW_MOD_SHIFT)
            std::exit(100 + i);
    // ======================

    // Se o usuário pressionar a tecla ESC, fechamos a janela.
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);

    // O código abaixo implementa a seguinte lógica:
    //   Se apertar tecla X       então g_AngleX += delta;
    //   Se apertar tecla shift+X então g_AngleX -= delta;
    //   Se apertar tecla Y       então g_AngleY += delta;
    //   Se apertar tecla shift+Y então g_AngleY -= delta;
    //   Se apertar tecla Z       então g_AngleZ += delta;
    //   Se apertar tecla shift+Z então g_AngleZ -= delta;

    float delta = 3.141592 / 16; // 22.5 graus, em radianos.

    if (key == GLFW_KEY_X && action == GLFW_PRESS)
    {
        g_AngleX += (mod & GLFW_MOD_SHIFT) ? -delta : delta;
    }

    if (key == GLFW_KEY_Y && action == GLFW_PRESS)
    {
        g_AngleY += (mod & GLFW_MOD_SHIFT) ? -delta : delta;
    }
    if (key == GLFW_KEY_Z && action == GLFW_PRESS)
    {
        g_AngleZ += (mod & GLFW_MOD_SHIFT) ? -delta : delta;
    }

    // Se o usuário apertar a tecla espaço, resetamos os ângulos de Euler para zero.
    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
    {
        g_AngleX = 0.0f;
        g_AngleY = 0.0f;
        g_AngleZ = 0.0f;
        g_ForearmAngleX = 0.0f;
        g_ForearmAngleZ = 0.0f;
        g_TorsoPositionX = 0.0f;
        g_TorsoPositionY = 0.0f;
    }

    // Se o usuário apertar a tecla P, utilizamos projeção perspectiva.
    if (key == GLFW_KEY_P && action == GLFW_PRESS)
    {
        g_UsePerspectiveProjection = true;
    }

    // Se o usuário apertar a tecla O, utilizamos projeção ortográfica.
    if (key == GLFW_KEY_O && action == GLFW_PRESS)
    {
        g_UsePerspectiveProjection = false;
    }

    // Se o usuário apertar a tecla H, fazemos um "toggle" do texto informativo mostrado na tela.
    if (key == GLFW_KEY_H && action == GLFW_PRESS)
    {
        g_ShowInfoText = !g_ShowInfoText;
    }

    // Se o usuário apertar a tecla R, recarregamos os shaders dos arquivos "shader_fragment.glsl" e "shader_vertex.glsl".
    if (key == GLFW_KEY_R && action == GLFW_PRESS)
    {
        LoadShadersFromFiles();
        fprintf(stdout,"Shaders recarregados!\n");
        fflush(stdout);
    }

        if (key == GLFW_KEY_W)
    {
        if (action == GLFW_PRESS)
            press_W = true;
        else if (action == GLFW_RELEASE)
            press_W = false;
    }
    if (key == GLFW_KEY_S)
    {
        if (action == GLFW_PRESS)
            press_S = true;
        else if (action == GLFW_RELEASE)
            press_S = false;
    }
    if (key == GLFW_KEY_A)
    {
        if (action == GLFW_PRESS)
            press_A = true;
        else if (action == GLFW_RELEASE)
            press_A = false;
    }
    if (key == GLFW_KEY_D)
    {
        if (action == GLFW_PRESS)
            press_D = true;
        else if (action == GLFW_RELEASE)
            press_D = false;
    }

    // a tecla G inicia um round
    if (key == GLFW_KEY_G && action == GLFW_PRESS && !g_isRoundActive)
    {
        g_isRoundActive = true;
        g_currentRound++;
        g_enemiesToSpawnForRound = 5 + g_currentRound * 2;
        printf("Iniciando Round %d com %d inimigos!\n", g_currentRound, g_enemiesToSpawnForRound);
    }

    if (key == GLFW_KEY_1 && action == GLFW_PRESS)
    {
        glm::vec3 pos = GetCursorWorldPosition(window);
        printf("%.1f, %.1f, %.1f",pos.x, pos.y, pos.z);
        if (pos.y > -998.0f)
        {
            auto new_tower = g_shop.buy("CannonTower_V1", g_playerMoney, pos);
            if (new_tower) {
                g_towers.push_back(std::move(new_tower));
                printf("Torre de Canhão comprada na posição (%.1f, %.1f)\n", pos.x, pos.z);
            } else {
                printf("Dinheiro insuficiente para comprar Canhao!\n");
            }
        }
    }

    if (key == GLFW_KEY_2 && action == GLFW_PRESS)
    {
        glm::vec3 pos = GetCursorWorldPosition(window);
        if (pos.y > -998.0f)
        {
            auto new_tower = g_shop.buy("Farm", g_playerMoney, pos);
            if (new_tower) {
                g_towers.push_back(std::move(new_tower));
                printf("Torre de Farm comprada na posição (%.1f, %.1f)\n", pos.x, pos.z);
            } else {
                printf("Dinheiro insuficiente para comprar Canhao!\n");
            }
        }
    }
     if (key == GLFW_KEY_3 && action == GLFW_PRESS)
    {
        glm::vec3 pos = GetCursorWorldPosition(window);
        if (pos.y > -998.0f)
        {
            auto new_tower = g_shop.buy("RocketTower", g_playerMoney, pos);
            if (new_tower) {
                g_towers.push_back(std::move(new_tower));
                printf("Torre de Rocket comprada na posição (%.1f, %.1f)\n", pos.x, pos.z);
            } else {
                printf("Dinheiro insuficiente para comprar Canhao!\n");
            }
        }
    }

    if (key == GLFW_KEY_4 && action == GLFW_PRESS)
    {
        glm::vec3 pos = GetCursorWorldPosition(window);
        if (pos.y > -998.0f)
        {
            auto new_tower = g_shop.buy("MortarTower", g_playerMoney, pos);
            if (new_tower) {
                g_towers.push_back(std::move(new_tower));
                printf("Torre de Rocket comprada na posição (%.1f, %.1f)\n", pos.x, pos.z);
            } else {
                printf("Dinheiro insuficiente para comprar Canhao!\n");
            }
        }
    }

    if (key == GLFW_KEY_5 && action == GLFW_PRESS)
    {
        glm::vec3 pos = GetCursorWorldPosition(window);
        if (pos.y > -998.0f)
        {
            auto new_tower = g_shop.buy("SlowTower", g_playerMoney, pos);
            if (new_tower) {
                g_towers.push_back(std::move(new_tower));
                printf("Torre de Rocket comprada na posição (%.1f, %.1f)\n", pos.x, pos.z);
            } else {
                printf("Dinheiro insuficiente para comprar Canhao!\n");
            }
        }
    }
   // Pressionar 'V' para vender
    if (key == GLFW_KEY_V && action == GLFW_PRESS && g_selectedTower)
    {
        SellSelectedTower(); // Chama a função que você já tem
    }

    // Pressionar 'F' para alternar a câmera
    if (key == GLFW_KEY_F && action == GLFW_PRESS && g_selectedTower)
    {
        ToggleTowerFOV(); // Chama a função que você já tem
    }

    if (key == GLFW_KEY_Q && action == GLFW_PRESS && g_selectedTower != nullptr)
    {
        if (g_selectedTower->targeting && !g_targetingFactories.empty())
        {
            // Pega o índice atual e calcula o próximo, dando a volta na lista
            int nextIndex = (g_selectedTower->targetingModeIndex + 1) % g_targetingFactories.size();
            
            // Cria o novo componente de mira usando a fábrica da lista
            g_selectedTower->targeting = g_targetingFactories[nextIndex]();
            
            // Atualiza o índice na torre
            g_selectedTower->targetingModeIndex = nextIndex;

            printf("Novo modo de foco da torre: %s\n", g_selectedTower->targeting->getModeName().c_str());
        }
    }

    // Pressionar 'ESC' para fechar a janela ou o jogo
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        if (g_isTowerInfoWindowOpen) {
            g_isTowerInfoWindowOpen = false;
            g_selectedTower = nullptr;
            g_cameraMode = CameraMode::ORBIT; // Sempre volta para a câmera normal
        } else {
            glfwSetWindowShouldClose(window, GL_TRUE);
        }
    }

}

// Definimos o callback para impressão de erros da GLFW no terminal
void ErrorCallback(int error, const char* description)
{
    fprintf(stderr, "ERROR: GLFW: %s\n", description);
}

// Esta função recebe um vértice com coordenadas de modelo p_model e passa o
// mesmo por todos os sistemas de coordenadas armazenados nas matrizes model,
// view, e projection; e escreve na tela as matrizes e pontos resultantes
// dessas transformações.
void TextRendering_ShowModelViewProjection(
    GLFWwindow* window,
    glm::mat4 projection,
    glm::mat4 view,
    glm::mat4 model,
    glm::vec4 p_model
)
{
    if ( !g_ShowInfoText )
        return;

    glm::vec4 p_world = model*p_model;
    glm::vec4 p_camera = view*p_world;
    glm::vec4 p_clip = projection*p_camera;
    glm::vec4 p_ndc = p_clip / p_clip.w;

    float pad = TextRendering_LineHeight(window);

    TextRendering_PrintString(window, " Model matrix             Model     In World Coords.", -1.0f, 1.0f-pad, 1.0f);
    TextRendering_PrintMatrixVectorProduct(window, model, p_model, -1.0f, 1.0f-2*pad, 1.0f);

    TextRendering_PrintString(window, "                                        |  ", -1.0f, 1.0f-6*pad, 1.0f);
    TextRendering_PrintString(window, "                            .-----------'  ", -1.0f, 1.0f-7*pad, 1.0f);
    TextRendering_PrintString(window, "                            V              ", -1.0f, 1.0f-8*pad, 1.0f);

    TextRendering_PrintString(window, " View matrix              World     In Camera Coords.", -1.0f, 1.0f-9*pad, 1.0f);
    TextRendering_PrintMatrixVectorProduct(window, view, p_world, -1.0f, 1.0f-10*pad, 1.0f);

    TextRendering_PrintString(window, "                                        |  ", -1.0f, 1.0f-14*pad, 1.0f);
    TextRendering_PrintString(window, "                            .-----------'  ", -1.0f, 1.0f-15*pad, 1.0f);
    TextRendering_PrintString(window, "                            V              ", -1.0f, 1.0f-16*pad, 1.0f);

    TextRendering_PrintString(window, " Projection matrix        Camera                    In NDC", -1.0f, 1.0f-17*pad, 1.0f);
    TextRendering_PrintMatrixVectorProductDivW(window, projection, p_camera, -1.0f, 1.0f-18*pad, 1.0f);

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);

    glm::vec2 a = glm::vec2(-1, -1);
    glm::vec2 b = glm::vec2(+1, +1);
    glm::vec2 p = glm::vec2( 0,  0);
    glm::vec2 q = glm::vec2(width, height);

    glm::mat4 viewport_mapping = Matrix(
        (q.x - p.x)/(b.x-a.x), 0.0f, 0.0f, (b.x*p.x - a.x*q.x)/(b.x-a.x),
        0.0f, (q.y - p.y)/(b.y-a.y), 0.0f, (b.y*p.y - a.y*q.y)/(b.y-a.y),
        0.0f , 0.0f , 1.0f , 0.0f ,
        0.0f , 0.0f , 0.0f , 1.0f
    );

    TextRendering_PrintString(window, "                                                       |  ", -1.0f, 1.0f-22*pad, 1.0f);
    TextRendering_PrintString(window, "                            .--------------------------'  ", -1.0f, 1.0f-23*pad, 1.0f);
    TextRendering_PrintString(window, "                            V                           ", -1.0f, 1.0f-24*pad, 1.0f);

    TextRendering_PrintString(window, " Viewport matrix           NDC      In Pixel Coords.", -1.0f, 1.0f-25*pad, 1.0f);
    TextRendering_PrintMatrixVectorProductMoreDigits(window, viewport_mapping, p_ndc, -1.0f, 1.0f-26*pad, 1.0f);
}

// Escrevemos na tela os ângulos de Euler definidos nas variáveis globais
// g_AngleX, g_AngleY, e g_AngleZ.
void TextRendering_ShowEulerAngles(GLFWwindow* window)
{
    if ( !g_ShowInfoText )
        return;

    float pad = TextRendering_LineHeight(window);

    char buffer[80];
    snprintf(buffer, 80, "Euler Angles rotation matrix = Z(%.2f)*Y(%.2f)*X(%.2f)\n", g_AngleZ, g_AngleY, g_AngleX);

    TextRendering_PrintString(window, buffer, -1.0f+pad/10, -1.0f+2*pad/10, 1.0f);
}

// Escrevemos na tela qual matriz de projeção está sendo utilizada.
void TextRendering_ShowProjection(GLFWwindow* window)
{
    if ( !g_ShowInfoText )
        return;

    float lineheight = TextRendering_LineHeight(window);
    float charwidth = TextRendering_CharWidth(window);

    if ( g_UsePerspectiveProjection )
        TextRendering_PrintString(window, "Perspective", 1.0f-13*charwidth, -1.0f+2*lineheight/10, 1.0f);
    else
        TextRendering_PrintString(window, "Orthographic", 1.0f-13*charwidth, -1.0f+2*lineheight/10, 1.0f);
}

// Escrevemos na tela o número de quadros renderizados por segundo (frames per
// second).
void TextRendering_ShowFramesPerSecond(GLFWwindow* window)
{
    if ( !g_ShowInfoText )
        return;

    // Variáveis estáticas (static) mantém seus valores entre chamadas
    // subsequentes da função!
    static float old_seconds = (float)glfwGetTime();
    static int   ellapsed_frames = 0;
    static char  buffer[20] = "?? fps";
    static int   numchars = 7;

    ellapsed_frames += 1;

    // Recuperamos o número de segundos que passou desde a execução do programa
    float seconds = (float)glfwGetTime();

    // Número de segundos desde o último cálculo do fps
    float ellapsed_seconds = seconds - old_seconds;

    if ( ellapsed_seconds > 1.0f )
    {
        numchars = snprintf(buffer, 20, "%.2f fps", ellapsed_frames / ellapsed_seconds);
    
        old_seconds = seconds;
        ellapsed_frames = 0;
    }

    float lineheight = TextRendering_LineHeight(window);
    float charwidth = TextRendering_CharWidth(window);

    TextRendering_PrintString(window, buffer, 1.0f-(numchars + 1)*charwidth, 1.0f-lineheight, 1.0f);
}

// Função para debugging: imprime no terminal todas informações de um modelo
// geométrico carregado de um arquivo ".obj".
// Veja: https://github.com/syoyo/tinyobjloader/blob/22883def8db9ef1f3ffb9b404318e7dd25fdbb51/loader_example.cc#L98
void PrintObjModelInfo(ObjModel* model)
{
  const tinyobj::attrib_t                & attrib    = model->attrib;
  const std::vector<tinyobj::shape_t>    & shapes    = model->shapes;
  const std::vector<tinyobj::material_t> & materials = model->materials;

  printf("# of vertices  : %d\n", (int)(attrib.vertices.size() / 3));
  printf("# of normals   : %d\n", (int)(attrib.normals.size() / 3));
  printf("# of texcoords : %d\n", (int)(attrib.texcoords.size() / 2));
  printf("# of shapes    : %d\n", (int)shapes.size());
  printf("# of materials : %d\n", (int)materials.size());

  for (size_t v = 0; v < attrib.vertices.size() / 3; v++) {
    printf("  v[%ld] = (%f, %f, %f)\n", static_cast<long>(v),
           static_cast<const double>(attrib.vertices[3 * v + 0]),
           static_cast<const double>(attrib.vertices[3 * v + 1]),
           static_cast<const double>(attrib.vertices[3 * v + 2]));
  }

  for (size_t v = 0; v < attrib.normals.size() / 3; v++) {
    printf("  n[%ld] = (%f, %f, %f)\n", static_cast<long>(v),
           static_cast<const double>(attrib.normals[3 * v + 0]),
           static_cast<const double>(attrib.normals[3 * v + 1]),
           static_cast<const double>(attrib.normals[3 * v + 2]));
  }

  for (size_t v = 0; v < attrib.texcoords.size() / 2; v++) {
    printf("  uv[%ld] = (%f, %f)\n", static_cast<long>(v),
           static_cast<const double>(attrib.texcoords[2 * v + 0]),
           static_cast<const double>(attrib.texcoords[2 * v + 1]));
  }

  // For each shape
  for (size_t i = 0; i < shapes.size(); i++) {
    printf("shape[%ld].name = %s\n", static_cast<long>(i),
           shapes[i].name.c_str());
    printf("Size of shape[%ld].indices: %lu\n", static_cast<long>(i),
           static_cast<unsigned long>(shapes[i].mesh.indices.size()));

    size_t index_offset = 0;

    assert(shapes[i].mesh.num_face_vertices.size() ==
           shapes[i].mesh.material_ids.size());

    printf("shape[%ld].num_faces: %lu\n", static_cast<long>(i),
           static_cast<unsigned long>(shapes[i].mesh.num_face_vertices.size()));

    // For each face
    for (size_t f = 0; f < shapes[i].mesh.num_face_vertices.size(); f++) {
      size_t fnum = shapes[i].mesh.num_face_vertices[f];

      printf("  face[%ld].fnum = %ld\n", static_cast<long>(f),
             static_cast<unsigned long>(fnum));

      // For each vertex in the face
      for (size_t v = 0; v < fnum; v++) {
        tinyobj::index_t idx = shapes[i].mesh.indices[index_offset + v];
        printf("    face[%ld].v[%ld].idx = %d/%d/%d\n", static_cast<long>(f),
               static_cast<long>(v), idx.vertex_index, idx.normal_index,
               idx.texcoord_index);
      }

      printf("  face[%ld].material_id = %d\n", static_cast<long>(f),
             shapes[i].mesh.material_ids[f]);

      index_offset += fnum;
    }

    printf("shape[%ld].num_tags: %lu\n", static_cast<long>(i),
           static_cast<unsigned long>(shapes[i].mesh.tags.size()));
    for (size_t t = 0; t < shapes[i].mesh.tags.size(); t++) {
      printf("  tag[%ld] = %s ", static_cast<long>(t),
             shapes[i].mesh.tags[t].name.c_str());
      printf(" ints: [");
      for (size_t j = 0; j < shapes[i].mesh.tags[t].intValues.size(); ++j) {
        printf("%ld", static_cast<long>(shapes[i].mesh.tags[t].intValues[j]));
        if (j < (shapes[i].mesh.tags[t].intValues.size() - 1)) {
          printf(", ");
        }
      }
      printf("]");

      printf(" floats: [");
      for (size_t j = 0; j < shapes[i].mesh.tags[t].floatValues.size(); ++j) {
        printf("%f", static_cast<const double>(
                         shapes[i].mesh.tags[t].floatValues[j]));
        if (j < (shapes[i].mesh.tags[t].floatValues.size() - 1)) {
          printf(", ");
        }
      }
      printf("]");

      printf(" strings: [");
      for (size_t j = 0; j < shapes[i].mesh.tags[t].stringValues.size(); ++j) {
        printf("%s", shapes[i].mesh.tags[t].stringValues[j].c_str());
        if (j < (shapes[i].mesh.tags[t].stringValues.size() - 1)) {
          printf(", ");
        }
      }
      printf("]");
      printf("\n");
    }
  }

  for (size_t i = 0; i < materials.size(); i++) {
    printf("material[%ld].name = %s\n", static_cast<long>(i),
           materials[i].name.c_str());
    printf("  material.Ka = (%f, %f ,%f)\n",
           static_cast<const double>(materials[i].ambient[0]),
           static_cast<const double>(materials[i].ambient[1]),
           static_cast<const double>(materials[i].ambient[2]));
    printf("  material.Kd = (%f, %f ,%f)\n",
           static_cast<const double>(materials[i].diffuse[0]),
           static_cast<const double>(materials[i].diffuse[1]),
           static_cast<const double>(materials[i].diffuse[2]));
    printf("  material.Ks = (%f, %f ,%f)\n",
           static_cast<const double>(materials[i].specular[0]),
           static_cast<const double>(materials[i].specular[1]),
           static_cast<const double>(materials[i].specular[2]));
    printf("  material.Tr = (%f, %f ,%f)\n",
           static_cast<const double>(materials[i].transmittance[0]),
           static_cast<const double>(materials[i].transmittance[1]),
           static_cast<const double>(materials[i].transmittance[2]));
    printf("  material.Ke = (%f, %f ,%f)\n",
           static_cast<const double>(materials[i].emission[0]),
           static_cast<const double>(materials[i].emission[1]),
           static_cast<const double>(materials[i].emission[2]));
    printf("  material.Ns = %f\n",
           static_cast<const double>(materials[i].shininess));
    printf("  material.Ni = %f\n", static_cast<const double>(materials[i].ior));
    printf("  material.dissolve = %f\n",
           static_cast<const double>(materials[i].dissolve));
    printf("  material.illum = %d\n", materials[i].illum);
    printf("  material.map_Ka = %s\n", materials[i].ambient_texname.c_str());
    printf("  material.map_Kd = %s\n", materials[i].diffuse_texname.c_str());
    printf("  material.map_Ks = %s\n", materials[i].specular_texname.c_str());
    printf("  material.map_Ns = %s\n",
           materials[i].specular_highlight_texname.c_str());
    printf("  material.map_bump = %s\n", materials[i].bump_texname.c_str());
    printf("  material.map_d = %s\n", materials[i].alpha_texname.c_str());
    printf("  material.disp = %s\n", materials[i].displacement_texname.c_str());
    printf("  <<PBR>>\n");
    printf("  material.Pr     = %f\n", materials[i].roughness);
    printf("  material.Pm     = %f\n", materials[i].metallic);
    printf("  material.Ps     = %f\n", materials[i].sheen);
    printf("  material.Pc     = %f\n", materials[i].clearcoat_thickness);
    printf("  material.Pcr    = %f\n", materials[i].clearcoat_thickness);
    printf("  material.aniso  = %f\n", materials[i].anisotropy);
    printf("  material.anisor = %f\n", materials[i].anisotropy_rotation);
    printf("  material.map_Ke = %s\n", materials[i].emissive_texname.c_str());
    printf("  material.map_Pr = %s\n", materials[i].roughness_texname.c_str());
    printf("  material.map_Pm = %s\n", materials[i].metallic_texname.c_str());
    printf("  material.map_Ps = %s\n", materials[i].sheen_texname.c_str());
    printf("  material.norm   = %s\n", materials[i].normal_texname.c_str());
    std::map<std::string, std::string>::const_iterator it(
        materials[i].unknown_parameter.begin());
    std::map<std::string, std::string>::const_iterator itEnd(
        materials[i].unknown_parameter.end());

    for (; it != itEnd; it++) {
      printf("  material.%s = %s\n", it->first.c_str(), it->second.c_str());
    }
    printf("\n");
  }
}

// set makeprg=cd\ ..\ &&\ make\ run\ >/dev/null
// vim: set spell spelllang=pt_br :

