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
#include <stdexcept>
#include <algorithm>

// Headers das bibliotecas OpenGL
#include <glad/glad.h>   // Criação de contexto OpenGL 3.3
#include <GLFW/glfw3.h>  // Criação de janelas do sistema operacional

// Headers da biblioteca GLM: criação de matrizes e vetores.
#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>
#include <glm/gtc/type_ptr.hpp>

// Headers locais, definidos na pasta "include/"
#include "rendering.hpp"
#include "textrendering.hpp"
#include "utils.hpp"
#include "matrices.hpp"
#include "bezier.hpp"
#include "collisions.h"

// Headers das estruturas
#include "EnemyManager.h"
#include "Enemy.h"
#include "Shop.h"
#include "TowerBlueprint.h"
#include "Tower.h"
#include "Path.h"

// Declaração de funções utilizadas para pilha de matrizes de modelagem.
void PushMatrix(glm::mat4 M);
void PopMatrix(glm::mat4& M);

void LoadContext();
void UnloadContext();

// Declaração de funções callback para comunicação com o sistema operacional e interação do
// usuário. Veja mais comentários nas definições das mesmas, abaixo.
void FramebufferSizeCallback(GLFWwindow* window, int width, int height);
void ErrorCallback(int error, const char* description);
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode);
void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void CursorPosCallback(GLFWwindow* window, double xpos, double ypos);
void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset);

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

GLuint g_gouraud_vertex_shader_id;
GLuint g_phong_vertex_shader_id;

GLuint g_gouraud_fragment_shader_id;
GLuint g_gouraud_fragment_shader_proj_esfer_id;
GLuint g_gouraud_fragment_shader_proj_planar_id;

GLuint g_phong_fragment_shader_id;
GLuint g_phong_fragment_shader_proj_esfer_id;
GLuint g_phong_fragment_shader_proj_planar_id;

GLuint g_floor_plane_fragment_shader_id;
GLuint g_range_indicator_fragment_shader_id;

GLuint g_gouraud_gpu_program_id;
GLuint g_gouraud_gpu_program_proj_esfer_id;
GLuint g_gouraud_gpu_program_proj_planar_id;

GLuint g_phong_gpu_program_id;
GLuint g_phong_gpu_program_proj_esfer_id;
GLuint g_phong_gpu_program_proj_planar_id;

GLuint g_floor_plane_gpu_program_id;
GLuint g_range_indicator_gpu_program_id;

// pos inicial
glm::vec4 g_camera_position_c  = glm::vec4(0.0f,1.0f,3.5f,1.0f);
glm::vec4 g_camera_position_c_ant =  g_camera_position_c;
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
float g_playerMoney = 750.0f;
int   g_playerLives = 5; 
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

struct SpawnEvent {
    float timeToNextSpawn; // Tempo de espera após o inimigo anterior
    EnemyAttribute type;   // Tipo do inimigo (Normal/Resistant, Fast, Flying)
    // Você pode adicionar mais campos aqui, como vida extra, etc.
};

// Descreve uma onda completa, que é uma sequência de eventos de spawn
struct Wave {
    std::vector<SpawnEvent> spawns;
};

// Lista global que define as ondas para cada round do jogo
std::vector<Wave> g_roundWaves;

void SetupGame() {
    printf("Configurando o jogo...\n");

    // --- caminho dos inimigos é uma curva bezier ---
    std::vector<glm::vec4> controlPoints = {
        {-20.0f, -1.0f, 0.0f, 1.0f}, {-5.0f, -1.0f, 15.0f, 1.0f},
        {10.0f, -1.0f, 5.0f, 1.0f}, {-10.0f, -1.0f, 10.0f, 1.0f},
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

    g_roundWaves.clear();

    // Inimigos normais e simples
    Wave round1;
    for (int i = 0; i < 10; ++i) {
        round1.spawns.push_back({1.5f, EnemyAttribute::RESISTANT}); // 10 inimigos resistentes, com 1.5s entre eles
    }
    g_roundWaves.push_back(round1);

    // RInimigos um pouco mais rápidos e misturados
    Wave round2;
    for (int i = 0; i < 8; ++i) {
        round2.spawns.push_back({1.0f, EnemyAttribute::RESISTANT});
    }
    for (int i = 0; i < 5; ++i) {
        round2.spawns.push_back({0.8f, EnemyAttribute::FAST}); // Adiciona 5 inimigos rápidos no final
    }
    g_roundWaves.push_back(round2);

    // Introdução de inimigos voadores
    Wave round3;
    for (int i = 0; i < 10; ++i) {
        round3.spawns.push_back({1.2f, EnemyAttribute::RESISTANT});
        if (i % 3 == 0) { // A cada 3 inimigos de chão, vem um voador
            round3.spawns.push_back({0.5f, EnemyAttribute::FLYING});
        }
    }
    g_roundWaves.push_back(round3);

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
    cannonV1_bp.upgradeCost = 100;
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
    cannonV2_bp.upgradeCost = 150;
    cannonV2_bp.nextUpgradeName = "CannonTower_V3"; 
    g_shop.registerTower(cannonV2_bp);

    TowerBlueprint cannonV3_bp;
    cannonV3_bp.name = "CannonTower_V3";
    cannonV3_bp.modelName = "the_cannon_tower";
    cannonV3_bp.cost = 0;
    cannonV3_bp.range = 12.5f;
    cannonV3_bp.cooldown = 0.4f;
    cannonV3_bp.targetingFactory = makeFirst;
    cannonV3_bp.shootingFactory = [](){ return std::unique_ptr<ProjectileShot>(new ProjectileShot(70, 0)); };
    cannonV3_bp.passiveFactory = nullptr; 
    g_shop.registerTower(cannonV3_bp);

    // --- Torre de Foguete ---
    TowerBlueprint rocketV1_bp;
    rocketV1_bp.name = "RocketTower_V1";
    rocketV1_bp.modelName = "the_rocket_tower";
    rocketV1_bp.canTargetFlying = true;
    rocketV1_bp.cost = 175;
    rocketV1_bp.range = 10.0f;
    rocketV1_bp.cooldown = 3.0f;
    rocketV1_bp.targetingFactory = makeFirst;
    rocketV1_bp.shootingFactory = [](){ return std::unique_ptr<SplashDamageShot>(new SplashDamageShot(50.0f, 25.0f, 3.0f)); };
    rocketV1_bp.passiveFactory = nullptr;
    rocketV1_bp.upgradeCost = 150;
    rocketV1_bp.nextUpgradeName = "RocketTower_V2";  
    g_shop.registerTower(rocketV1_bp);
    
    TowerBlueprint rocketV2_bp;
    rocketV2_bp.name = "RocketTower_V2";
    rocketV2_bp.modelName = "the_rocket_tower";
    rocketV2_bp.canTargetFlying = true;
    rocketV2_bp.cost = 0;
    rocketV2_bp.range = 12.0f;
    rocketV2_bp.cooldown = 2.7f;
    rocketV2_bp.targetingFactory = makeFirst;
    rocketV2_bp.shootingFactory = [](){ return std::unique_ptr<SplashDamageShot>(new SplashDamageShot(75.0f, 30.0f, 3.5f)); };
    rocketV2_bp.passiveFactory = nullptr; 
    rocketV2_bp.upgradeCost = 200;
    rocketV2_bp.nextUpgradeName = "RocketTower_V3"; 
    g_shop.registerTower(rocketV2_bp);

    TowerBlueprint rocketV3_bp;
    rocketV3_bp.name = "RocketTower_V3";
    rocketV3_bp.modelName = "the_rocket_tower";
    rocketV3_bp.canTargetFlying = true;
    rocketV3_bp.cost = 0;
    rocketV3_bp.range = 15.0f;
    rocketV3_bp.cooldown = 2.5f;
    rocketV3_bp.targetingFactory = makeFirst;
    rocketV3_bp.shootingFactory = [](){ return std::unique_ptr<SplashDamageShot>(new SplashDamageShot(100.0f, 50.0f, 4.0f)); };
    rocketV3_bp.passiveFactory = nullptr;
    g_shop.registerTower(rocketV3_bp);


    // --- Torre de Morteiro ---
    TowerBlueprint mortarV1_bp;
    mortarV1_bp.name = "MortarTower_V1";
    mortarV1_bp.modelName = "the_mortar_tower"; 
    mortarV1_bp.cost = 250;
    mortarV1_bp.range = 18.0f;
    mortarV1_bp.cooldown = 5.0f;
    mortarV1_bp.targetingFactory = makeFirst; 
    mortarV1_bp.shootingFactory = [](){ return std::unique_ptr<SplashDamageShot>(new SplashDamageShot(100.0f, 80.0f, 4.0f)); };
    mortarV1_bp.passiveFactory = nullptr;
    mortarV1_bp.upgradeCost = 200;
    mortarV1_bp.nextUpgradeName = "MortarTower_V2";
    g_shop.registerTower(mortarV1_bp);


    TowerBlueprint mortarV2_bp;
    mortarV2_bp.name = "MortarTower_V2";
    mortarV2_bp.modelName = "the_mortar_tower"; 
    mortarV2_bp.cost = 0; 
    mortarV2_bp.range = 20.0f; 
    mortarV2_bp.cooldown = 4.5f; 
    mortarV2_bp.targetingFactory = makeFirst;
    mortarV2_bp.shootingFactory = [](){ return std::unique_ptr<SplashDamageShot>(new SplashDamageShot(150.0f, 120.0f, 5.0f)); }; 
    mortarV2_bp.passiveFactory = nullptr;
    mortarV2_bp.upgradeCost = 350;
    mortarV2_bp.nextUpgradeName = "MortarTower_V3";
    g_shop.registerTower(mortarV2_bp);
    

    TowerBlueprint mortarV3_bp;
    mortarV3_bp.name = "MortarTower_V3";
    mortarV3_bp.modelName = "the_mortar_tower";
    mortarV3_bp.cost = 0;
    mortarV3_bp.range = 25.0f; 
    mortarV3_bp.cooldown = 6.0f; 
    mortarV3_bp.targetingFactory = makeFirst; 
    mortarV3_bp.shootingFactory = [](){ return std::unique_ptr<SplashDamageShot>(new SplashDamageShot(300.0f, 250.0f, 6.5f)); }; 
    mortarV3_bp.passiveFactory = nullptr;
    g_shop.registerTower(mortarV3_bp);


    // --- Farm de Dinheiro ---
    TowerBlueprint farmV1_bp;
    farmV1_bp.name = "Farm_V1";
    farmV1_bp.modelName = "the_farm";
    farmV1_bp.cost = 125;
    farmV1_bp.passiveFactory = [](){ return std::unique_ptr<GenerateIncome>(new GenerateIncome(50)); };
    farmV1_bp.upgradeCost = 200;
    farmV1_bp.nextUpgradeName = "Farm_V2"; 
    g_shop.registerTower(farmV1_bp);

    TowerBlueprint farmV2_bp;
    farmV2_bp.name = "Farm_V2"; 
    farmV2_bp.modelName = "the_farm";
    farmV2_bp.cost = 0;
    farmV2_bp.passiveFactory = [](){ return std::unique_ptr<GenerateIncome>(new GenerateIncome(120)); }; 
    farmV2_bp.upgradeCost = 350;
    farmV2_bp.nextUpgradeName = "Farm_V3"; 
    g_shop.registerTower(farmV2_bp);

    TowerBlueprint farmV3_bp;
    farmV3_bp.name = "Farm_V3";
    farmV3_bp.modelName = "the_farm";
    farmV3_bp.cost = 0;
    farmV3_bp.passiveFactory = [](){ return std::unique_ptr<GenerateIncome>(new GenerateIncome(250)); };
    g_shop.registerTower(farmV3_bp);


    // --- Torre de slow ---
    TowerBlueprint ice_towerV1_bp;
    ice_towerV1_bp.name = "SlowTower_V1";
    ice_towerV1_bp.modelName = "the_slow_tower";
    ice_towerV1_bp.cost = 75;
    ice_towerV1_bp.range = 7.0f;
    ice_towerV1_bp.cooldown = 2.0f;
    ice_towerV1_bp.targetingFactory = makeNearest;
    ice_towerV1_bp.shootingFactory = [](){ return std::unique_ptr<IceShot>(new IceShot(2.0f)); }; 
    ice_towerV1_bp.passiveFactory = nullptr;
    ice_towerV1_bp.upgradeCost = 100;
    ice_towerV1_bp.nextUpgradeName = "SlowTower_V2";
    g_shop.registerTower(ice_towerV1_bp);

    TowerBlueprint ice_towerV2_bp;
    ice_towerV2_bp.name = "SlowTower_V2";
    ice_towerV2_bp.modelName = "the_slow_tower";
    ice_towerV2_bp.cost = 0;
    ice_towerV2_bp.range = 8.0f;
    ice_towerV2_bp.cooldown = 1.5f; 
    ice_towerV2_bp.targetingFactory = makeNearest;
    ice_towerV2_bp.shootingFactory = [](){ return std::unique_ptr<IceShot>(new IceShot(3.0f)); }; 
    ice_towerV2_bp.passiveFactory = nullptr;
    ice_towerV2_bp.upgradeCost = 175;
    ice_towerV2_bp.nextUpgradeName = "SlowTower_V3";
    g_shop.registerTower(ice_towerV2_bp);
    

    TowerBlueprint ice_towerV3_bp;
    ice_towerV3_bp.name = "SlowTower_V3";
    ice_towerV3_bp.modelName = "the_slow_tower"; 
    ice_towerV3_bp.cost = 0;
    ice_towerV3_bp.range = 9.0f;
    ice_towerV3_bp.cooldown = 1.0f; 
    ice_towerV3_bp.targetingFactory = makeNearest;
    ice_towerV3_bp.shootingFactory = [](){ return std::unique_ptr<FullAoeIceShot>(new FullAoeIceShot(2.5f)); }; 
    ice_towerV3_bp.passiveFactory = nullptr;
    g_shop.registerTower(ice_towerV3_bp);

    printf("Setup do jogo concluído!\n");
}

void ResetGame() {
    printf("Fim de Jogo! A base foi destruída. Resetando para o início...\n");
    g_playerLives = 5;
    g_playerMoney = 500;
    g_towers.clear();
    g_enemyManager.clearAll();
    g_currentRound = 0;
    g_isRoundActive = false;
    g_selectedTower = nullptr;
    g_cameraMode = CameraMode::ORBIT;
}

void SellSelectedTower() {
    if (!g_selectedTower) return;

    // Encontra e remove a torre do vetor g_towers
    g_towers.erase(std::remove_if(g_towers.begin(), g_towers.end(), 
        [](const std::unique_ptr<Tower>& tower) {
            return tower.get() == g_selectedTower;
        }), 
        g_towers.end()
    );

    // Devolve 75% do custo da torre para o jogador 
    const auto& bp = g_shop.getBlueprint(g_selectedTower->blueprintName);
    if (bp) {
        g_playerMoney += bp->cost * 0.75f;
    }

    printf("Torre vendida!\n");
    g_selectedTower = nullptr;
    g_isTowerInfoWindowOpen = false;
}

void ToggleTowerFOV() {
    if (g_cameraMode == CameraMode::ORBIT && g_selectedTower != nullptr) {
        g_cameraMode = CameraMode::TOWER_FOV;
        printf("Câmera em modo Terceira Pessoa.\n");
    } else {
        g_cameraMode = CameraMode::ORBIT;
        printf("Câmera em modo Orbita.\n");
    }
}

// Converte a posição 2D do cursor na tela para uma posição 3D no chão do mundo
glm::vec3 GetCursorWorldPosition(GLFWwindow* window) {

    int width, height;
    glfwGetWindowSize(window, &width, &height);

    // Converte as coordenadas do cursorpara coordenadas normalizadas
    float x_ndc = (2.0f * (float)g_cursor_x) / width - 1.0f;
    float y_ndc = 1.0f - (2.0f * (float)g_cursor_y) / height;

    // Define um raio em espaço de recorte
    glm::vec4 ray_clip = glm::vec4(x_ndc, y_ndc, -1.0, 1.0);

    // Converte o raio do espaço de recorte para o espaço da câmera 
    glm::vec4 ray_eye = inverse(g_projection_matrix) * ray_clip;
    ray_eye = glm::vec4(ray_eye.x, ray_eye.y, -1.0, 0.0); 

    // Converte o raio do espaço da câmera para o espaço do mundo 
    glm::vec3 ray_world_dir = glm::normalize(glm::vec3(inverse(g_view_matrix) * ray_eye));

    // Calcula a interseção do raio com o plano do chão
    // Posição da câmera 
    glm::vec3 ray_origin = glm::vec3(inverse(g_view_matrix)[3]);
    
    // Se o raio não estiver apontando para baixo, não haverá interseção
    if (ray_world_dir.y >= 0.0f) {
        return glm::vec3(0.0f, -999.0f, 0.0f);
    }
    
    // Calcula a distância 't' até a interseção
    float t = (-1.0f - ray_origin.y) / ray_world_dir.y;

    // Retorna o ponto de interseção
    return ray_origin + t * ray_world_dir;
}

int main(int argc, char* argv[]) {
    // Inicializamos a biblioteca GLFW, utilizada para criar uma janela do
    // sistema operacional, onde poderemos renderizar com OpenGL.
    int success = glfwInit();
    if (!success) {
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
    if (!window) {
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

    // Carregamos o contexto de shaders e texturas
    LoadContext();

    // Construímos a representação de objetos geométricos através de malhas de triângulos
    const glm::vec3 Ka = glm::vec3(0.1f, 0.1f, 0.1f);
    const glm::vec3 Ks = glm::vec3(0.0f, 0.0f, 0.0f);
    const float q = 1;

    ObjModel spheremodel("../../data/sphere.obj");
    ComputeNormals(&spheremodel);
    Shape sphereShape(spheremodel, "the_sphere");

    SceneObject sphereObject(
        sphereShape,
        g_gouraud_gpu_program_proj_esfer_id,
        {1},
        Ka, Ks, q
    );

    SceneObject rangeIndicatorObject(
        sphereShape,
        g_range_indicator_gpu_program_id,
        {},
        Ka, Ks, q
    );


    ObjModel bunnymodel("../../data/bunny.obj");
    ComputeNormals(&bunnymodel);
    Shape bunnyShape(bunnymodel, "the_bunny");
    SceneObject bunnyObject(
        bunnyShape,
        g_phong_gpu_program_proj_planar_id,
        {0},
        Ka, Ks, q
    );

    ObjModel planeModel("../../data/plane.obj");
    ComputeNormals(&planeModel);
    Shape planeShape(planeModel, "the_plane");

    SceneObject floorPlaneObject(
        planeShape,
        g_floor_plane_gpu_program_id,
        {0},
        glm::vec3(0.15f, 0.15f, 0.15f), Ks, q
    );

    SceneObject stampObject(
        planeShape,
        g_phong_gpu_program_id,
        {10},
        Ka, Ks, q
    );

    ObjModel rocketmodel("../../data/rocket_tower.obj");
    ComputeNormals(&rocketmodel);
    Shape rocketShape(rocketmodel, "the_rocket_tower");
    SceneObject rocketObject(
        rocketShape,
        g_phong_gpu_program_id,
        {2},
        Ka, Ks, q
    );

    ObjModel farmmodel("../../data/farm.obj");
    ComputeNormals(&farmmodel);
    Shape farmShape(farmmodel, "the_farm");
    SceneObject farmObject(
        farmShape,
        g_phong_gpu_program_id,
        {3},
        Ka, Ks, q
    );

    ObjModel cannonmodel("../../data/cannon_tower.obj");
    ComputeNormals(&cannonmodel);
    Shape cannonShape(cannonmodel, "the_cannon_tower");
    SceneObject cannonObject(
        cannonShape,
        g_phong_gpu_program_id,
        {4},
        Ka, Ks, q
    );

    ObjModel mortarmodel("../../data/mortar_tower.obj");
    ComputeNormals(&mortarmodel);
    Shape mortarShape(mortarmodel, "the_mortar_tower");
    SceneObject mortarObject(
        mortarShape,
        g_phong_gpu_program_id,
        {5},
        Ka, Ks, q
    );

    ObjModel slowmodel("../../data/slow_tower.obj");
    ComputeNormals(&slowmodel);
    Shape slowShape(slowmodel, "the_slow_tower");
    SceneObject slowObject(
        slowShape,
        g_phong_gpu_program_id,
        {7},
        Ka, Ks, q
    );

    ObjModel portalmodel("../../data/portal.obj");
    ComputeNormals(&portalmodel);
    Shape portalShape(portalmodel, "the_portal");
    SceneObject portalObject(
        portalShape,
        g_phong_gpu_program_id,
        {8},
        Ka, Ks, q
    );

    ObjModel castlemodel("../../data/castle.obj");
    ComputeNormals(&castlemodel);
    Shape castleShape(castlemodel, "the_castle");
    SceneObject castleObject(
        castleShape,
        g_phong_gpu_program_id,
        {9},
        Ka, Ks, q
    );

    ObjModel *extraModel;
    Shape *extraShape;
    SceneObject *extraObject;

    if (argc >= 5) {
        extraModel = new ObjModel(argv[1]);
        extraShape = new Shape(*extraModel, argv[2]);
        extraObject = new SceneObject(
            *extraShape,
            argv[3], argv[4],
            {0},
            Ka, Ks, q
        );
    }

    // Inicializamos o código para renderização de texto.
    TextRendering_Init();

    // Habilitamos o Z-buffer. Veja slides 104-116 do documento Aula_09_Projecoes.pdf.
    glEnable(GL_DEPTH_TEST);

    // Habilitamos o Backface Culling. Veja slides 8-13 do documento Aula_02_Fundamentos_Matematicos.pdf, slides 23-34 do documento Aula_13_Clipping_and_Culling.pdf e slides 112-123 do documento Aula_14_Laboratorio_3_Revisao.pdf.
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    SetupGame();

    // Ficamos em um loop infinito, renderizando, até que o usuário feche a janela
    while (!glfwWindowShouldClose(window)) {
        // Computamos a posição da câmera utilizando coordenadas esféricas
        float current_time = (float)glfwGetTime();
        delta_t = current_time - prev_time;
        prev_time = current_time;

        if (g_isRoundActive) {
            static float spawnTimer = 1.0f; // Timer para controlar a velocidade de spawn
            spawnTimer -= delta_t;
            
            if (spawnTimer <= 0.0f && g_enemiesToSpawnForRound > 0) {
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

            int finished_count = g_enemyManager.updateAll(delta_t, g_playerMoney);
            if (finished_count > 0) {
                g_playerLives -= finished_count;
                printf("%d inimigo(s) alcancaram a base! Vidas restantes: %d\n", finished_count, g_playerLives);
            }

            if (g_playerLives <= 0) {
                ResetGame();
            }

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
        } else { // Pausa entre os rounds
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

        glm::mat4 view;
        glm::vec4 camera_position_c;
        glm::vec4 camera_view_vector;
        glm::vec4 camera_up_vector = glm::vec4(0.0f,1.0f,0.0f,0.0f);

        if (g_cameraMode == CameraMode::TOWER_FOV && g_selectedTower != nullptr) {
            glm::vec3 tower_pos = g_selectedTower->pos;
            float tower_rotation_y = g_selectedTower->currentYRotation;

            // A posição final da câmera é calculada subtraindo o deslocamento que criamos da posição da torre
            // com 4 unidades para trás e 2 unidades para cima.

            glm::vec3 offset = glm::vec3(sin(tower_rotation_y) * 4.0f, -2.0f, cos(tower_rotation_y) * 4.0f);
            camera_position_c = glm::vec4(tower_pos - offset, 1.0f);
            glm::vec4 camera_lookat_l = glm::vec4(tower_pos, 1.0f);
            camera_view_vector = camera_lookat_l - camera_position_c;
        } else {
            // Câmera orbital
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

            bool has_collided = CheckCameraCollision(g_camera_position_c, g_towers, -0.5f, 95.0f, 1.0f);
            
            if (!has_collided) {
                camera_position_c = g_camera_position_c;
                g_camera_position_c_ant = g_camera_position_c;
            }else{
                g_camera_position_c = g_camera_position_c_ant;
                camera_position_c = g_camera_position_c_ant;
            }
        }
        
        view = Matrix_Camera_View(camera_position_c, camera_view_vector, camera_up_vector);
        glm::mat4 projection;
        float nearplane = -0.1f;
        float farplane  = -299.0f;

        if (g_UsePerspectiveProjection) {
            float field_of_view = 3.141592 / 3.0f;
            projection = Matrix_Perspective(field_of_view, g_ScreenRatio, nearplane, farplane);
        } else {
            float t = 1.5f * g_CameraDistance / 2.5f;
            float b = -t;
            float r = t * g_ScreenRatio;
            float l = -r;
            projection = Matrix_Orthographic(l, r, b, t, nearplane, farplane); // Os planos são positivos aqui
        }

        glm::mat4 model = Matrix_Identity(); // Transformação identidade de modelagem
        g_view_matrix = view;
        g_projection_matrix = projection;

        const glm::vec4 light_source = glm::vec4(1.0f, 1.0f, 0.0f, 0.0f);
        const glm::vec3 light_color = glm::vec3(1.0f, 1.0f, 1.0f);
        const glm::vec3 ambient_color = glm::vec3(1.0f, 1.0f, 1.0f);

        glDepthFunc(GL_LEQUAL); 
        model = Matrix_Translate(camera_position_c.x, camera_position_c.y, camera_position_c.z) 
              * Matrix_Scale(95.0f, 95.0f, 95.0f);
        sphereObject.draw(
            model, view, projection,
            light_source, light_color, ambient_color
        );
        glDepthFunc(GL_LESS); 

        float path_length = g_enemyPath->getTotalLength();
        float stamp_spacing = 1.5f; // Distância entre cada carimbo de textura
        float stamp_size = 2.5f;    // Tamanho de cada carimbo

        // Percorre o caminho, desenhando um carimbo a cada 'stamp_spacing' unidades
        for (float dist = 0; dist < path_length; dist += stamp_spacing) {
            // Pega o parâmetro 't' e a posição 3D para a distância atual
            float t = g_enemyPath->getTForDistance(dist);
            glm::vec3 position = g_enemyPath->getPoint(t);

            // Calcula a direção do caminho nesse ponto para orientar o carimbo
            glm::vec3 tangent = glm::normalize(g_enemyPath->getPoint(t + 0.01f) - position);
            float path_angle = atan2(tangent.x, tangent.z);

            float rotation = 6.28f; 
            float offset_x = 0.5f;
            float offset_z = 0.5f;

            position.x += offset_x;
            position.z += offset_z;

            // Cria a matriz 'model' para este carimbo específico
            model = Matrix_Translate(position.x, position.y - 0.3f, position.z)
                  * Matrix_Rotate_Y(path_angle + rotation)
                  * Matrix_Scale(stamp_size, stamp_size, stamp_size);
                
            // Desenha um único "pedaço de terra"
            stampObject.draw(
                model, view, projection,
                light_source, light_color, ambient_color
            );
        }

        // Portal
        glm::vec3 portal_pos = g_enemyPath->getStartPoint();
        glm::vec3 portal_tangent = glm::normalize(g_enemyPath->getPoint(0.01f) - portal_pos);
        float portal_angle = atan2(portal_tangent.x, portal_tangent.z);

        model = Matrix_Translate(portal_pos.x, 0.0f, portal_pos.z)
              * Matrix_Rotate_Y(-portal_angle); 
        portalObject.draw(
            model, view, projection,
            light_source, light_color, ambient_color
        );

        // Castelo
        glm::vec3 castle_pos = g_enemyPath->getEndPoint();
        glm::vec3 castle_tangent = glm::normalize(castle_pos - g_enemyPath->getPoint(0.99f));
        float castle_angle = atan2(castle_tangent.x, castle_tangent.z);

        model = Matrix_Translate(castle_pos.x, 0.2f, castle_pos.z)
                * Matrix_Scale (2.0f, 2.0f, 2.0f)
                * Matrix_Rotate_Y(-castle_angle);
        castleObject.draw(
            model, view, projection,
            light_source, light_color, ambient_color
        );

        // Desenha o chão
        model = Matrix_Translate(0.0f, -1.5f, 0.0f)
              * Matrix_Scale(25.0f, 1.0f, 25.0f);
        floorPlaneObject.draw(
            model, view, projection,
            light_source, light_color, ambient_color
        );

        // Loop que desenha torre ao comprar
        for (const auto& tower : g_towers) {
            // Pega a posição da torre
            model = Matrix_Translate(tower->pos.x, tower->pos.y, tower->pos.z)
                  // Movimenta a torre na direção do foco do inimigo
                  * Matrix_Rotate_Y(tower->currentYRotation); 
            
            // Define o ID para o shader saber como colorir/texturizar
            if (tower->modelName == "the_cannon_tower") {
                cannonObject.draw(
                    model, view, projection,
                    light_source, light_color, ambient_color
                );
            } else if (tower->modelName == "the_farm") {
                farmObject.draw(
                    model, view, projection,
                    light_source, light_color, ambient_color
                );
            } else if (tower->modelName == "the_rocket_tower") {
                rocketObject.draw(
                    model, view, projection,
                    light_source, light_color, ambient_color
                );
            } else if (tower->modelName == "the_mortar_tower") {
                mortarObject.draw(
                    model, view, projection,
                    light_source, light_color, ambient_color
                );
            } else if (tower->modelName == "the_slow_tower") {
                slowObject.draw(
                    model, view, projection,
                    light_source, light_color, ambient_color
                );
            } 
        }

        auto enemy_pointers = g_enemyManager.getEnemyPointers();

        // Percorre a lista de inimigos e desenha cada um
        for (const auto* enemy : g_enemyManager.getEnemyPointers()) {
            // Efeito de Bobbing no andar do inimigo
            float time = (float)glfwGetTime();

            // Parâmetros da animação 
            float bobbing_speed = 4.0f;  
            float bobbing_height = 0.1f; 

            // Calcula o deslocamento vertical usando a função seno
            float y_offset = sin(time * bobbing_speed) * bobbing_height;

            // Posição base do inimigo
            glm::vec3 enemy_pos = enemy->hitbox.center;

            // Cria a matriz 'model' com a posição base + o deslocamento da animação
            model = Matrix_Translate(enemy_pos.x, enemy_pos.y + y_offset, enemy_pos.z)
                  * Matrix_Scale(enemy->hitbox.radius, enemy->hitbox.radius, enemy->hitbox.radius);

            bunnyObject.draw(
                model, view, projection,
                light_source, light_color, ambient_color
            );
        }

        // desenha o range da torre
        if (g_selectedTower != nullptr) {
            // Habilita a transparência
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            // Posição da esfera é o centro da torre selecionada
            model = Matrix_Translate(g_selectedTower->pos.x, g_selectedTower->pos.y, g_selectedTower->pos.z)
                    * Matrix_Scale(g_selectedTower->range, g_selectedTower->range, g_selectedTower->range);
            
            // CORREÇÃO: Desenha a geometria da esfera em vez do plano
            rangeIndicatorObject.draw(
                model, view, projection,
                light_source, light_color, ambient_color
            );
            
            // Desabilita a transparência para não afetar outros objetos
            glDisable(GL_BLEND);
        }


        char buffer[100];
        snprintf(buffer, sizeof(buffer), "Dinheiro: %.0f", g_playerMoney);
        TextRendering_PrintString(window, buffer, -0.95f, 0.95f);
        snprintf(buffer, sizeof(buffer), "Vida: %d", g_playerLives);
        TextRendering_PrintString(window, buffer, -0.95f, 0.90f);
        snprintf(buffer, sizeof(buffer), "Round: %d", g_currentRound);
        TextRendering_PrintString(window, buffer, -0.95f, 0.85f);

        if (!g_isRoundActive && g_currentRound < g_totalRounds) {
            snprintf(buffer, sizeof(buffer), "Pressione 'G' para iniciar o Round %d", g_currentRound + 1);
            TextRendering_PrintString(window, buffer, -0.4f, 0.0f);
        }

        // selecionamento da torre
        if (g_selectedTower) {
            float text_y = -0.40f;
            float line_height = 0.05f;

           // Mostra o nome da torre
            snprintf(buffer, sizeof(buffer), "Torre Selecionada: %s", g_selectedTower->blueprintName.c_str());
            TextRendering_PrintString(window, buffer, -0.95f, text_y);

            // Verifica se a torre pode atacar para mostrar Dano, Cooldown e Foco
            if (g_selectedTower->shooting) {
                
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

                // Mostra o foco atual da torre
                if (g_selectedTower->targeting) {
                    std::string mode_info = g_selectedTower->targeting->getModeName();
                    snprintf(buffer, sizeof(buffer), "- Foco: %s", mode_info.c_str());
                    TextRendering_PrintString(window, buffer, -0.95f, text_y - 4*line_height);
                }

            // Se for uma torre passiva (Farm), mostra sua habilidade
            } else if (g_selectedTower->passiveAbility) { 
                std::string passive_info = g_selectedTower->passiveAbility->getInfo();
                snprintf(buffer, sizeof(buffer), "- Habilidade: %s", passive_info.c_str());
                TextRendering_PrintString(window, buffer, -0.95f, text_y - 2*line_height);
            }

            const auto& current_bp = g_shop.getBlueprint(g_selectedTower->blueprintName);
            if (current_bp && !current_bp->nextUpgradeName.empty()) {
                //  Se houver um upgrade, pega o custo dele
                const auto& next_bp = g_shop.getBlueprint(current_bp->nextUpgradeName);
                if (next_bp) {
                    snprintf(buffer, sizeof(buffer), "[U] - Upgrade (Custo: %d)", current_bp->upgradeCost);
                    TextRendering_PrintString(window, buffer, -0.95f, text_y - 5 * line_height);
                }
            } else {
                TextRendering_PrintString(window, "[U] - Upgrade (Nivel Maximo)", -0.95f, text_y - 5 * line_height);
            }

            TextRendering_PrintString(window, "[V] - Vender", -0.95f, text_y - 6*line_height);
            TextRendering_PrintString(window, "[F] - Ver da Torre", -0.95f, text_y - 7*line_height);
            TextRendering_PrintString(window, "[Q] - Mudar Foco", -0.95f, text_y - 8*line_height);
        }



        if (g_ShowInfoText) {
            // Imprimimos na tela os ângulos de Euler que controlam a rotação do
            // terceiro cubo.
            TextRendering_ShowEulerAngles(window, g_AngleX, g_AngleY, g_AngleZ);

            // Imprimimos na informação sobre a matriz de projeção sendo utilizada.
            TextRendering_ShowProjection(window, g_UsePerspectiveProjection);

            // Imprimimos na tela informação sobre o número de quadros renderizados
            // por segundo (frames per second).
            TextRendering_ShowFramesPerSecond(window);
        }

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

// Função que pega a matriz M e guarda a mesma no topo da pilha
void PushMatrix(glm::mat4 M) {
    g_MatrixStack.push(M);
}

// Função que remove a matriz atualmente no topo da pilha e armazena a mesma na variável M
void PopMatrix(glm::mat4& M) {
    if (g_MatrixStack.empty()) {
        M = Matrix_Identity();
    } else {
        M = g_MatrixStack.top();
        g_MatrixStack.pop();
    }
}

void LoadContext() {
    // Carregamos os shaders de vértices e de fragmentos que serão utilizados
    // para renderização. Veja slides 180-200 do documento Aula_03_Rendering_Pipeline_Grafico.pdf.
    g_gouraud_vertex_shader_id = LoadShader_Vertex("../../shaders/gouraud/vertex.glsl");
    g_phong_vertex_shader_id = LoadShader_Vertex("../../shaders/phong/vertex.glsl");

    g_gouraud_fragment_shader_id = LoadShader_Fragment("../../shaders/gouraud/fragment.glsl");
    g_gouraud_fragment_shader_proj_esfer_id = LoadShader_Fragment("../../shaders/gouraud/fragment_proj_esfer.glsl");
    g_gouraud_fragment_shader_proj_planar_id = LoadShader_Fragment("../../shaders/gouraud/fragment_proj_planar.glsl");

    g_phong_fragment_shader_id = LoadShader_Fragment("../../shaders/phong/fragment.glsl");
    g_phong_fragment_shader_proj_esfer_id = LoadShader_Fragment("../../shaders/phong/fragment_proj_esfer.glsl");
    g_phong_fragment_shader_proj_planar_id = LoadShader_Fragment("../../shaders/phong/fragment_proj_planar.glsl");

    g_floor_plane_fragment_shader_id = LoadShader_Fragment("../../shaders/floor_plane_fragment.glsl");
    g_range_indicator_fragment_shader_id = LoadShader_Fragment("../../shaders/range_indicator_fragment.glsl");

    g_gouraud_gpu_program_id = CreateGpuProgram(g_gouraud_vertex_shader_id, g_gouraud_fragment_shader_id);
    g_gouraud_gpu_program_proj_esfer_id = CreateGpuProgram(g_gouraud_vertex_shader_id, g_gouraud_fragment_shader_proj_esfer_id);
    g_gouraud_gpu_program_proj_planar_id = CreateGpuProgram(g_gouraud_vertex_shader_id, g_gouraud_fragment_shader_proj_planar_id);

    g_phong_gpu_program_id = CreateGpuProgram(g_phong_vertex_shader_id, g_phong_fragment_shader_id);
    g_phong_gpu_program_proj_esfer_id = CreateGpuProgram(g_phong_vertex_shader_id, g_phong_fragment_shader_proj_esfer_id);
    g_phong_gpu_program_proj_planar_id = CreateGpuProgram(g_phong_vertex_shader_id, g_phong_fragment_shader_proj_planar_id);

    g_floor_plane_gpu_program_id = CreateGpuProgram(g_phong_vertex_shader_id, g_floor_plane_fragment_shader_id);
    g_range_indicator_gpu_program_id = CreateGpuProgram(g_phong_vertex_shader_id, g_range_indicator_fragment_shader_id);

    // Carregamos as imagens para serem utilizadas como textura
    LoadTextureImage("../../data/grama.jpg", 0);
    LoadTextureImage("../../data/tc-earth_nightmap_citylights.gif", 1);
    LoadTextureImage("../../data/rocket_tower.jpg", 2);
    LoadTextureImage("../../data/farm.jpg", 3);
    LoadTextureImage("../../data/cannon_tower.jpg", 4);
    LoadTextureImage("../../data/mortar_tower.jpg", 5);
    LoadTextureImage("../../data/ceu.jpg", 6);
    LoadTextureImage("../../data/slow_tower.jpg", 7);
    LoadTextureImage("../../data/portal.jpg", 8);
    LoadTextureImage("../../data/castle.jpg", 9);
    LoadTextureImage("../../data/caminho.jpg", 10);
}

void UnloadContext() {
    glDeleteShader(g_gouraud_vertex_shader_id);
    glDeleteShader(g_phong_vertex_shader_id);

    glDeleteShader(g_gouraud_fragment_shader_id);
    glDeleteShader(g_gouraud_fragment_shader_proj_esfer_id);
    glDeleteShader(g_gouraud_fragment_shader_proj_planar_id);

    glDeleteShader(g_floor_plane_fragment_shader_id);
    glDeleteShader(g_range_indicator_fragment_shader_id);

    glDeleteShader(g_phong_fragment_shader_id);
    glDeleteShader(g_phong_fragment_shader_proj_esfer_id);
    glDeleteShader(g_phong_fragment_shader_proj_planar_id);

    glDeleteProgram(g_gouraud_gpu_program_id);
    glDeleteProgram(g_gouraud_gpu_program_proj_esfer_id);
    glDeleteProgram(g_gouraud_gpu_program_proj_planar_id);

    glDeleteProgram(g_phong_gpu_program_id);
    glDeleteProgram(g_phong_gpu_program_proj_esfer_id);
    glDeleteProgram(g_phong_gpu_program_proj_planar_id);

    glDeleteProgram(g_floor_plane_gpu_program_id);
    glDeleteProgram(g_range_indicator_gpu_program_id);
}

// Definição da função que será chamada sempre que a janela do sistema
// operacional for redimensionada, por consequência alterando o tamanho do
// "framebuffer" (região de memória onde são armazenados os pixels da imagem).
void FramebufferSizeCallback(GLFWwindow* window, int width, int height) {
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
void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
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

        for (const auto& tower : g_towers) {
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

    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
        // Quando o usuário soltar o botão esquerdo do mouse, atualizamos a
        // variável abaixo para false.
        g_LeftMouseButtonPressed = false;
    }

    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
        // Se o usuário pressionou o botão esquerdo do mouse, guardamos a
        // posição atual do cursor nas variáveis g_LastCursorPosX e
        // g_LastCursorPosY.  Também, setamos a variável
        // g_RightMouseButtonPressed como true, para saber que o usuário está
        // com o botão esquerdo pressionado.
        glfwGetCursorPos(window, &g_LastCursorPosX, &g_LastCursorPosY);
        g_RightMouseButtonPressed = true;
    }

    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE) {
        // Quando o usuário soltar o botão esquerdo do mouse, atualizamos a
        // variável abaixo para false.
        g_RightMouseButtonPressed = false;
    }

    if (button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_PRESS) {
        // Se o usuário pressionou o botão esquerdo do mouse, guardamos a
        // posição atual do cursor nas variáveis g_LastCursorPosX e
        // g_LastCursorPosY.  Também, setamos a variável
        // g_MiddleMouseButtonPressed como true, para saber que o usuário está
        // com o botão esquerdo pressionado.
        glfwGetCursorPos(window, &g_LastCursorPosX, &g_LastCursorPosY);
        g_MiddleMouseButtonPressed = true;
    }

    if (button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_RELEASE) {
        // Quando o usuário soltar o botão esquerdo do mouse, atualizamos a
        // variável abaixo para false.
        g_MiddleMouseButtonPressed = false;
    }
}

// Função callback chamada sempre que o usuário movimentar o cursor do mouse em
// cima da janela OpenGL.
void CursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
    // Abaixo executamos o seguinte: caso o botão esquerdo do mouse esteja
    // pressionado, computamos quanto que o mouse se movimento desde o último
    // instante de tempo, e usamos esta movimentação para atualizar os
    // parâmetros que definem a posição da câmera dentro da cena virtual.
    // Assim, temos que o usuário consegue controlar a câmera.
    g_cursor_x = xpos;
    g_cursor_y = ypos;

    if (g_LeftMouseButtonPressed) {
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

    if (g_RightMouseButtonPressed) {
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

    if (g_MiddleMouseButtonPressed) {
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
void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
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
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mod) {
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

    if (key == GLFW_KEY_X && action == GLFW_PRESS) {
        g_AngleX += (mod & GLFW_MOD_SHIFT) ? -delta : delta;
    }

    if (key == GLFW_KEY_Y && action == GLFW_PRESS) {
        g_AngleY += (mod & GLFW_MOD_SHIFT) ? -delta : delta;
    }
    if (key == GLFW_KEY_Z && action == GLFW_PRESS) {
        g_AngleZ += (mod & GLFW_MOD_SHIFT) ? -delta : delta;
    }

    // Se o usuário apertar a tecla espaço, resetamos os ângulos de Euler para zero.
    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
        g_AngleX = 0.0f;
        g_AngleY = 0.0f;
        g_AngleZ = 0.0f;
        g_ForearmAngleX = 0.0f;
        g_ForearmAngleZ = 0.0f;
        g_TorsoPositionX = 0.0f;
        g_TorsoPositionY = 0.0f;
    }

    // Se o usuário apertar a tecla P, utilizamos projeção perspectiva.
    if (key == GLFW_KEY_P && action == GLFW_PRESS) {
        g_UsePerspectiveProjection = true;
    }

    // Se o usuário apertar a tecla O, utilizamos projeção ortográfica.
    if (key == GLFW_KEY_O && action == GLFW_PRESS) {
        g_UsePerspectiveProjection = false;
    }

    // Se o usuário apertar a tecla H, fazemos um "toggle" do texto informativo mostrado na tela.
    if (key == GLFW_KEY_H && action == GLFW_PRESS) {
        g_ShowInfoText = !g_ShowInfoText;
    }

    // Se o usuário apertar a tecla R, recarregamos os shaders dos arquivos "shader_fragment.glsl" e "shader_vertex.glsl".
    if (key == GLFW_KEY_R && action == GLFW_PRESS) {
        UnloadContext();
        LoadContext();

        fprintf(stdout,"Shaders recarregados!\n");
        fflush(stdout);
    }

    if (key == GLFW_KEY_W) {
        if (action == GLFW_PRESS)
            press_W = true;
        else if (action == GLFW_RELEASE)
            press_W = false;
    }

    if (key == GLFW_KEY_S) {
        if (action == GLFW_PRESS)
            press_S = true;
        else if (action == GLFW_RELEASE)
            press_S = false;
    }

    if (key == GLFW_KEY_A) {
        if (action == GLFW_PRESS)
            press_A = true;
        else if (action == GLFW_RELEASE)
            press_A = false;
    }

    if (key == GLFW_KEY_D) {
        if (action == GLFW_PRESS)
            press_D = true;
        else if (action == GLFW_RELEASE)
            press_D = false;
    }

    // a tecla G inicia um round
    if (key == GLFW_KEY_G && action == GLFW_PRESS && !g_isRoundActive) {
        g_isRoundActive = true;
        g_currentRound++;
        g_enemiesToSpawnForRound = 5 + g_currentRound * 2;
        printf("Iniciando Round %d com %d inimigos!\n", g_currentRound, g_enemiesToSpawnForRound);
    }

    if (key == GLFW_KEY_1 && action == GLFW_PRESS) {
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

    if (key == GLFW_KEY_2 && action == GLFW_PRESS) {
        glm::vec3 pos = GetCursorWorldPosition(window);
        if (pos.y > -998.0f) {
            auto new_tower = g_shop.buy("Farm_V1", g_playerMoney, pos);
            if (new_tower) {
                g_towers.push_back(std::move(new_tower));
                printf("Torre de Farm comprada na posição (%.1f, %.1f)\n", pos.x, pos.z);
            } else {
                printf("Dinheiro insuficiente para comprar Canhao!\n");
            }
        }
    }

    if (key == GLFW_KEY_3 && action == GLFW_PRESS) {
        glm::vec3 pos = GetCursorWorldPosition(window);
        if (pos.y > -998.0f) {
            auto new_tower = g_shop.buy("RocketTower_V1", g_playerMoney, pos);
            if (new_tower) {
                g_towers.push_back(std::move(new_tower));
                printf("Torre de Rocket comprada na posição (%.1f, %.1f)\n", pos.x, pos.z);
            } else {
                printf("Dinheiro insuficiente para comprar Canhao!\n");
            }
        }
    }

    if (key == GLFW_KEY_4 && action == GLFW_PRESS) {
        glm::vec3 pos = GetCursorWorldPosition(window);
        if (pos.y > -998.0f) {
            auto new_tower = g_shop.buy("MortarTower_V1", g_playerMoney, pos);
            if (new_tower) {
                g_towers.push_back(std::move(new_tower));
                printf("Torre de Rocket comprada na posição (%.1f, %.1f)\n", pos.x, pos.z);
            } else {
                printf("Dinheiro insuficiente para comprar Canhao!\n");
            }
        }
    }

    if (key == GLFW_KEY_5 && action == GLFW_PRESS) {
        glm::vec3 pos = GetCursorWorldPosition(window);
        if (pos.y > -998.0f) {
            auto new_tower = g_shop.buy("SlowTower_V1", g_playerMoney, pos);
            if (new_tower) {
                g_towers.push_back(std::move(new_tower));
                printf("Torre de Rocket comprada na posição (%.1f, %.1f)\n", pos.x, pos.z);
            } else {
                printf("Dinheiro insuficiente para comprar Canhao!\n");
            }
        }
    }
   // Pressionar 'V' para vender
    if (key == GLFW_KEY_V && action == GLFW_PRESS && g_selectedTower) {
        SellSelectedTower(); // Chama a função que você já tem
    }

    // Pressionar 'F' para alternar a câmera
    if (key == GLFW_KEY_F && action == GLFW_PRESS && g_selectedTower) {
        ToggleTowerFOV(); // Chama a função que você já tem
    }

    if (key == GLFW_KEY_Q && action == GLFW_PRESS && g_selectedTower != nullptr) {
        if (g_selectedTower->targeting && !g_targetingFactories.empty()) {
            // Pega o índice atual e calcula o próximo, dando a volta na lista
            int nextIndex = (g_selectedTower->targetingModeIndex + 1) % g_targetingFactories.size();
            
            // Cria o novo componente de mira usando a fábrica da lista
            g_selectedTower->targeting = g_targetingFactories[nextIndex]();
            
            // Atualiza o índice na torre
            g_selectedTower->targetingModeIndex = nextIndex;

            printf("Novo modo de foco da torre: %s\n", g_selectedTower->targeting->getModeName().c_str());
        }
    }

    if (key == GLFW_KEY_U && action == GLFW_PRESS) {
        printf("Tentando upgrade da torre '%s'...\n", g_selectedTower->blueprintName.c_str());

        // encontrar o unique_ptr correto no vetor g_towers
        for (auto& tower_ptr : g_towers) {
            if (tower_ptr.get() == g_selectedTower) {
                auto upgraded_tower = g_shop.upgrade(*tower_ptr, g_playerMoney);
                if (upgraded_tower) {
                    printf("Upgrade bem-sucedido!\n");
                    g_selectedTower = upgraded_tower.get();
                    tower_ptr = std::move(upgraded_tower);
                } else {
                    printf("Upgrade falhou! (Sem upgrade disponivel ou dinheiro insuficiente)\n");
                }
                break; // Sai do loop 'for' pois já encontramos a torre
            }
        }
    }

    // Pressionar 'ESC' para fechar a janela ou o jogo
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
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
