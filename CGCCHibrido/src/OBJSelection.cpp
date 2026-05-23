#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <filesystem>

namespace fs = std::filesystem;

// GLAD
#include <glad/glad.h>

// GLFW
#include <GLFW/glfw3.h>

// STB Image
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

// GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

struct PointLight
{
    glm::vec3 position;
    glm::vec3 color;
    float intensity;
    bool enabled;
};

struct MeshData
{
    GLuint VAO;
    int vertexCount;
    std::string name;
    glm::vec3 position;
    glm::vec3 scale;
    glm::vec3 color;
    glm::vec3 rotation;  // rotation angles in degrees (x, y, z)
    GLuint textureID;
    bool hasTexture;
    std::string textureName;
};

std::vector<MeshData> meshes;
int selectedMeshIndex = 0;

// Sistema de iluminação
PointLight keyLight;
PointLight fillLight;
PointLight backLight;

bool moveForward = false;
bool moveBackward = false;
bool moveLeft = false;
bool moveRight = false;
bool moveUp = false;
bool moveDown = false;
bool scaleUp = false;
bool scaleDown = false;

bool rotateX = false;
bool rotateY = false;
bool rotateZ = false;
bool rotatePos = false;
bool rotateNeg = false;
bool wireframeMode = false;

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
int setupShader();
int loadSimpleOBJ(const std::string& filePath, int& nVertices, std::string& textureName);
GLuint loadTexture(const std::string& filePath);
fs::path locateAssetFolder(const fs::path& exePath);

fs::path locateAssetFolder(const fs::path& exePath)
{
    fs::path candidates[] = {
        exePath.parent_path() / "assets" / "Modelos3D",
        exePath.parent_path().parent_path() / "assets" / "Modelos3D",
        fs::current_path() / "assets" / "Modelos3D",
        fs::current_path().parent_path() / "assets" / "Modelos3D"
    };

    for (const auto& candidate : candidates)
    {
        if (fs::exists(candidate))
            return fs::canonical(candidate);
    }

    return candidates[1];
}

const GLchar* vertexShaderSource = "#version 450\n"
"layout(location = 0) in vec3 position;\n"
"layout(location = 1) in vec2 texCoord;\n"
"layout(location = 2) in vec3 normal;\n"
"uniform mat4 projection;\n"
"uniform mat4 view;\n"
"uniform mat4 model;\n"
"out vec2 TexCoord;\n"
"out vec3 FragPos;\n"
"out vec3 Normal;\n"
"void main()\n"
"{\n"
"    gl_Position = projection * view * model * vec4(position, 1.0);\n"
"    FragPos = vec3(model * vec4(position, 1.0));\n"
"    Normal = normalize(mat3(transpose(inverse(model))) * normal);\n"
"    TexCoord = texCoord;\n"
"}\0";

const GLchar* fragmentShaderSource = "#version 450\n"
"in vec2 TexCoord;\n"
"in vec3 FragPos;\n"
"in vec3 Normal;\n"
"uniform sampler2D textureSampler;\n"
"uniform vec4 objectColor;\n"
"uniform bool useTexture;\n"
"uniform vec3 viewPos;\n"
"uniform vec3 lightPos[3];\n"
"uniform vec3 lightColor[3];\n"
"uniform float lightIntensity[3];\n"
"uniform bool lightEnabled[3];\n"
"out vec4 color;\n"
"void main()\n"
"{\n"
"    vec4 baseColor;\n"
"    if (useTexture)\n"
"        baseColor = texture(textureSampler, TexCoord);\n"
"    else\n"
"        baseColor = objectColor;\n"
"    \n"
"    vec3 norm = normalize(Normal);\n"
"    vec3 viewDir = normalize(viewPos - FragPos);\n"
"    vec3 result = vec3(0.0);\n"
"    \n"
"    // Ambient light\n"
"    float ambientStrength = 0.1;\n"
"    result += ambientStrength * baseColor.rgb;\n"
"    \n"
"    // Process each light\n"
"    for(int i = 0; i < 3; i++)\n"
"    {\n"
"        if(!lightEnabled[i]) continue;\n"
"        \n"
"        vec3 lightDir = normalize(lightPos[i] - FragPos);\n"
"        float distance = length(lightPos[i] - FragPos);\n"
"        \n"
"        // Attenuation: 1 / (1 + 0.1*d + 0.02*d^2)\n"
"        float attenuation = 1.0 / (1.0 + 0.1 * distance + 0.02 * distance * distance);\n"
"        \n"
"        // Diffuse reflection\n"
"        float diff = max(dot(norm, lightDir), 0.0);\n"
"        vec3 diffuse = diff * lightColor[i] * baseColor.rgb * lightIntensity[i] * attenuation;\n"
"        \n"
"        // Specular reflection\n"
"        float specularStrength = 0.5;\n"
"        vec3 reflectDir = reflect(-lightDir, norm);\n"
"        float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);\n"
"        vec3 specular = specularStrength * spec * lightColor[i] * lightIntensity[i] * attenuation;\n"
"        \n"
"        result += diffuse + specular;\n"
"    }\n"
"    \n"
"    color = vec4(result, baseColor.a);\n"
"}\0";

int main(int argc, char** argv)
{
    if (!glfwInit())
    {
        std::cerr << "Falha ao inicializar GLFW" << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(1000, 1000, "OBJ Selection - Natália", nullptr, nullptr);
    if (!window)
    {
        std::cerr << "Falha ao criar janela GLFW" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, key_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        glfwTerminate();
        return -1;
    }

    keyLight.position = glm::vec3(3.0f, 4.0f, 3.0f);
    keyLight.color = glm::vec3(1.0f, 1.0f, 1.0f);
    keyLight.intensity = 1.2f;
    keyLight.enabled = true;

    fillLight.position = glm::vec3(-3.0f, 2.0f, 2.0f);
    fillLight.color = glm::vec3(0.8f, 0.9f, 1.0f);
    fillLight.intensity = 0.6f;
    fillLight.enabled = true;

    backLight.position = glm::vec3(0.0f, 3.0f, -4.0f);
    backLight.color = glm::vec3(0.6f, 0.7f, 1.0f);
    backLight.intensity = 0.5f;
    backLight.enabled = true;

    auto exePath = fs::absolute(argv[0]);
    fs::path assetsFolder = locateAssetFolder(exePath);
    if (!fs::exists(assetsFolder))
    {
        std::cerr << "Pasta de assets não encontrada: " << assetsFolder.string() << std::endl;
        glfwTerminate();
        return -1;
    }

    std::cout << "Pasta de assets: " << assetsFolder.string() << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "OBJ Selection Example" << std::endl;
    std::cout << "Controles de Objeto:" << std::endl;
    std::cout << "  TAB: Selecionar próximo objeto" << std::endl;
    std::cout << "  W/S: Mover objeto selecionado para frente/trás" << std::endl;
    std::cout << "  A/D: Mover objeto selecionado para esquerda/direita" << std::endl;
    std::cout << "  Q/E: Mover objeto selecionado para cima/baixo" << std::endl;
    std::cout << "  K/L: Diminuir/Aumentar escala do objeto selecionado" << std::endl;
    std::cout << "  X/Y/Z: Selecionar eixo de rotação" << std::endl;
    std::cout << "  R/T: Girar objeto selecionado (+/-15 graus no eixo)" << std::endl;
    std::cout << "Controles de Iluminação:" << std::endl;
    std::cout << "  1: Alternar Key Light (luz principal)" << std::endl;
    std::cout << "  2: Alternar Fill Light (luz de preenchimento)" << std::endl;
    std::cout << "  3: Alternar Back Light (luz de fundo)" << std::endl;
    std::cout << "  ESC: Sair" << std::endl;
    std::cout << "========================================" << std::endl;

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);

    GLuint shaderProgram = setupShader();
    glUseProgram(shaderProgram);

    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 100.0f);
    glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 0.0f, 5.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    GLint projLoc = glGetUniformLocation(shaderProgram, "projection");
    GLint viewLoc = glGetUniformLocation(shaderProgram, "view");
    std::cout << "Uniform locations: projection=" << projLoc << " view=" << viewLoc << std::endl;
    if (projLoc != -1)
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
    if (viewLoc != -1)
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

    std::vector<fs::path> objFiles = {
        fs::path(assetsFolder) / "Cube.obj",
        fs::path(assetsFolder) / "Suzanne.obj",
        fs::path(assetsFolder) / "SuzanneSubdiv1.obj"
    };
    std::vector<glm::vec3> positions = { glm::vec3(-2.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(2.0f, 0.0f, 0.0f) };
    std::vector<glm::vec3> baseColors = { glm::vec3(0.7f, 0.5f, 0.3f), glm::vec3(0.5f, 0.8f, 0.5f), glm::vec3(0.6f, 0.6f, 0.9f) };

    for (size_t i = 0; i < objFiles.size(); i++)
    {
        int nVertices;
        std::string textureName;
        std::string objPath = objFiles[i].string();
        GLuint vao = loadSimpleOBJ(objPath, nVertices, textureName);
        if (vao == 0 || nVertices <= 0)
        {
            std::cerr << "Falha ao carregar OBJ: " << objPath << std::endl;
            continue;
        }

        MeshData mesh;
        mesh.VAO = vao;
        mesh.vertexCount = nVertices;
        mesh.name = objFiles[i].filename().string();
        mesh.position = positions[i];
        mesh.scale = glm::vec3(0.6f);
        mesh.color = baseColors[i];
        mesh.rotation = glm::vec3(0.0f, 0.0f, 0.0f);
        mesh.textureID = 0;
        mesh.hasTexture = false;
        mesh.textureName = textureName;

        if (!textureName.empty())
        {
            std::string texturePath = (objFiles[i].parent_path() / textureName).string();
            mesh.textureID = loadTexture(texturePath);
            mesh.hasTexture = true;
            std::cout << "Textura carregada: " << texturePath << std::endl;
        }

        std::cout << "Mesh carregada: " << mesh.name << " | vertices=" << mesh.vertexCount << " | texture=" << mesh.hasTexture << std::endl;
        meshes.push_back(mesh);
    }

    if (meshes.empty())
    {
        std::cerr << "Nenhum objeto carregado. Verifique o caminho dos arquivos OBJ." << std::endl;
        glfwTerminate();
        return -1;
    }

    selectedMeshIndex = 0;
    std::cout << "Objeto selecionado: " << meshes[selectedMeshIndex].name << std::endl;

    GLint modelLoc = glGetUniformLocation(shaderProgram, "model");
    GLint colorLoc = glGetUniformLocation(shaderProgram, "objectColor");
    GLint textureLoc = glGetUniformLocation(shaderProgram, "textureSampler");
    GLint useTextureLoc = glGetUniformLocation(shaderProgram, "useTexture");
    GLint viewPosLoc = glGetUniformLocation(shaderProgram, "viewPos");
    GLint lightPosLoc[3], lightColorLoc[3], lightIntensityLoc[3], lightEnabledLoc[3];

    for (int i = 0; i < 3; i++)
    {
        lightPosLoc[i] = glGetUniformLocation(shaderProgram, ("lightPos[" + std::to_string(i) + "]").c_str());
        lightColorLoc[i] = glGetUniformLocation(shaderProgram, ("lightColor[" + std::to_string(i) + "]").c_str());
        lightIntensityLoc[i] = glGetUniformLocation(shaderProgram, ("lightIntensity[" + std::to_string(i) + "]").c_str());
        lightEnabledLoc[i] = glGetUniformLocation(shaderProgram, ("lightEnabled[" + std::to_string(i) + "]").c_str());
    }

    if (textureLoc != -1)
    {
        glUniform1i(textureLoc, 0);
    }

    glEnable(GL_DEPTH_TEST);
    bool debugFirstDraw = true;
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        MeshData& selectedMesh = meshes[selectedMeshIndex];

        if (moveForward) selectedMesh.position.z -= 0.02f;
        if (moveBackward) selectedMesh.position.z += 0.02f;
        if (moveLeft) selectedMesh.position.x -= 0.02f;
        if (moveRight) selectedMesh.position.x += 0.02f;
        if (moveUp) selectedMesh.position.y += 0.02f;
        if (moveDown) selectedMesh.position.y -= 0.02f;

        if (scaleUp) selectedMesh.scale += glm::vec3(0.01f);
        if (scaleDown) selectedMesh.scale -= glm::vec3(0.01f);
        selectedMesh.scale = glm::max(selectedMesh.scale, glm::vec3(0.1f));

        if (rotatePos)
        {
            if (rotateX) selectedMesh.rotation.x += 5.0f;
            if (rotateY) selectedMesh.rotation.y += 5.0f;
            if (rotateZ) selectedMesh.rotation.z += 5.0f;
        }
        if (rotateNeg)
        {
            if (rotateX) selectedMesh.rotation.x -= 5.0f;
            if (rotateY) selectedMesh.rotation.y -= 5.0f;
            if (rotateZ) selectedMesh.rotation.z -= 5.0f;
        }

        // Normalizar ângulos para 0-360
        selectedMesh.rotation.x = fmod(selectedMesh.rotation.x, 360.0f);
        selectedMesh.rotation.y = fmod(selectedMesh.rotation.y, 360.0f);
        selectedMesh.rotation.z = fmod(selectedMesh.rotation.z, 360.0f);

        glClearColor(0.15f, 0.15f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::vec3 viewPos = glm::vec3(0.0f, 0.0f, 5.0f);
        glUniform3fv(viewPosLoc, 1, glm::value_ptr(viewPos));

        glUniform3fv(lightPosLoc[0], 1, glm::value_ptr(keyLight.position));
        glUniform3fv(lightColorLoc[0], 1, glm::value_ptr(keyLight.color));
        glUniform1f(lightIntensityLoc[0], keyLight.intensity);
        glUniform1i(lightEnabledLoc[0], keyLight.enabled);

        glUniform3fv(lightPosLoc[1], 1, glm::value_ptr(fillLight.position));
        glUniform3fv(lightColorLoc[1], 1, glm::value_ptr(fillLight.color));
        glUniform1f(lightIntensityLoc[1], fillLight.intensity);
        glUniform1i(lightEnabledLoc[1], fillLight.enabled);

        glUniform3fv(lightPosLoc[2], 1, glm::value_ptr(backLight.position));
        glUniform3fv(lightColorLoc[2], 1, glm::value_ptr(backLight.color));
        glUniform1f(lightIntensityLoc[2], backLight.intensity);
        glUniform1i(lightEnabledLoc[2], backLight.enabled);

        for (size_t i = 0; i < meshes.size(); i++)
        {
            MeshData& mesh = meshes[i];
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, mesh.position);
            model = glm::rotate(model, glm::radians(mesh.rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
            model = glm::rotate(model, glm::radians(mesh.rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
            model = glm::rotate(model, glm::radians(mesh.rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
            model = glm::scale(model, mesh.scale);

            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

            if ((int)i == selectedMeshIndex)
            {
                glUniform4f(colorLoc, 1.0f, 1.0f, 0.2f, 1.0f);
            }
            else
            {
                glUniform4f(colorLoc, mesh.color.r, mesh.color.g, mesh.color.b, 1.0f);
            }

            if (useTextureLoc != -1)
            {
                glUniform1i(useTextureLoc, mesh.hasTexture ? 1 : 0);
            }

            if (mesh.hasTexture)
            {
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, mesh.textureID);
            }
            else
            {
                glBindTexture(GL_TEXTURE_2D, 0);
            }

            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

            if (debugFirstDraw)
            {
                std::cout << "Renderizando mesh: " << mesh.name << " vertices=" << mesh.vertexCount << " hasTexture=" << mesh.hasTexture << "\n";
            }

            glBindVertexArray(mesh.VAO);
            glDrawArrays(GL_TRIANGLES, 0, mesh.vertexCount);
        }

        if (debugFirstDraw)
            debugFirstDraw = false;

        glBindVertexArray(0);
        glfwSwapBuffers(window);
    }

    glfwTerminate();
    return 0;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);

    if (key == GLFW_KEY_TAB && action == GLFW_PRESS)
    {
        selectedMeshIndex = (selectedMeshIndex + 1) % meshes.size();
        std::cout << "Objeto selecionado: " << meshes[selectedMeshIndex].name << std::endl;
    }

    if (key == GLFW_KEY_1 && action == GLFW_PRESS)
    {
        keyLight.enabled = !keyLight.enabled;
        std::cout << "Key Light: " << (keyLight.enabled ? "ATIVADA" : "DESATIVADA") << std::endl;
    }
    if (key == GLFW_KEY_2 && action == GLFW_PRESS)
    {
        fillLight.enabled = !fillLight.enabled;
        std::cout << "Fill Light: " << (fillLight.enabled ? "ATIVADA" : "DESATIVADA") << std::endl;
    }
    if (key == GLFW_KEY_3 && action == GLFW_PRESS)
    {
        backLight.enabled = !backLight.enabled;
        std::cout << "Back Light: " << (backLight.enabled ? "ATIVADA" : "DESATIVADA") << std::endl;
    }

    if (key == GLFW_KEY_W)
    {
        if (action == GLFW_PRESS) moveForward = true;
        else if (action == GLFW_RELEASE) moveForward = false;
    }
    if (key == GLFW_KEY_S)
    {
        if (action == GLFW_PRESS) moveBackward = true;
        else if (action == GLFW_RELEASE) moveBackward = false;
    }
    if (key == GLFW_KEY_A)
    {
        if (action == GLFW_PRESS) moveLeft = true;
        else if (action == GLFW_RELEASE) moveLeft = false;
    }
    if (key == GLFW_KEY_D)
    {
        if (action == GLFW_PRESS) moveRight = true;
        else if (action == GLFW_RELEASE) moveRight = false;
    }
    if (key == GLFW_KEY_Q)
    {
        if (action == GLFW_PRESS) moveUp = true;
        else if (action == GLFW_RELEASE) moveUp = false;
    }
    if (key == GLFW_KEY_E)
    {
        if (action == GLFW_PRESS) moveDown = true;
        else if (action == GLFW_RELEASE) moveDown = false;
    }
    if (key == GLFW_KEY_L)
    {
        if (action == GLFW_PRESS) scaleUp = true;
        else if (action == GLFW_RELEASE) scaleUp = false;
    }
    if (key == GLFW_KEY_K)
    {
        if (action == GLFW_PRESS) scaleDown = true;
        else if (action == GLFW_RELEASE) scaleDown = false;
    }

    if (key == GLFW_KEY_X && action == GLFW_PRESS)
    {
        rotateX = true;
        rotateY = false;
        rotateZ = false;
    }
    if (key == GLFW_KEY_Y && action == GLFW_PRESS)
    {
        rotateX = false;
        rotateY = true;
        rotateZ = false;
    }
    if (key == GLFW_KEY_Z && action == GLFW_PRESS)
    {
        rotateX = false;
        rotateY = false;
        rotateZ = true;
    }

    if (key == GLFW_KEY_R)
    {
        if (action == GLFW_PRESS) rotatePos = true;
        else if (action == GLFW_RELEASE) rotatePos = false;
    }
    if (key == GLFW_KEY_T)
    {
        if (action == GLFW_PRESS) rotateNeg = true;
        else if (action == GLFW_RELEASE) rotateNeg = false;
    }

    if (key == GLFW_KEY_P && action == GLFW_PRESS)
    {
        wireframeMode = !wireframeMode;
        glPolygonMode(GL_FRONT_AND_BACK, wireframeMode ? GL_LINE : GL_FILL);
        std::cout << "Wireframe: " << (wireframeMode ? "ON" : "OFF") << std::endl;
    }
}

std::string loadMTL(const std::string& filePath)
{
    std::ifstream mtlFile(filePath);
    std::string line;
    std::string textureName = "";
    if (!mtlFile.is_open())
    {
        return "";
    }

    while (std::getline(mtlFile, line))
    {
        std::istringstream ss(line);
        std::string token;
        ss >> token;
        if (token == "map_Kd")
        {
            ss >> textureName;
            break;
        }
    }

    return textureName;
}

int loadSimpleOBJ(const std::string& filePath, int& nVertices, std::string& textureName)
{
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec2> texCoords;
    std::vector<glm::vec3> normals;
    std::vector<GLfloat> vBuffer;

    std::ifstream file(filePath);
    if (!file.is_open())
    {
        std::cerr << "Erro ao abrir arquivo OBJ: " << filePath << std::endl;
        nVertices = 0;
        return 0;
    }

    std::string line;
    while (std::getline(file, line))
    {
        std::istringstream ss(line);
        std::string word;
        ss >> word;

        if (word == "mtllib")
        {
            std::string mtlFile;
            ss >> mtlFile;
            std::string mtlPath = filePath.substr(0, filePath.find_last_of("/\\") + 1) + mtlFile;
            textureName = loadMTL(mtlPath);
        }
        else if (word == "v")
        {
            glm::vec3 vertex;
            ss >> vertex.x >> vertex.y >> vertex.z;
            vertices.push_back(vertex);
        }
        else if (word == "vt")
        {
            glm::vec2 texCoord;
            ss >> texCoord.s >> texCoord.t;
            texCoords.push_back(texCoord);
        }
        else if (word == "vn")
        {
            glm::vec3 normal;
            ss >> normal.x >> normal.y >> normal.z;
            normals.push_back(normal);
        }
        else if (word == "f")
        {
            std::string vertexToken;
            while (ss >> vertexToken)
            {
                int vi = 0, ti = 0, ni = 0;
                std::istringstream ssToken(vertexToken);
                std::string index;
                if (std::getline(ssToken, index, '/')) vi = !index.empty() ? std::stoi(index) - 1 : 0;
                if (std::getline(ssToken, index, '/')) ti = !index.empty() ? std::stoi(index) - 1 : 0;
                if (std::getline(ssToken, index, '/')) ni = !index.empty() ? std::stoi(index) - 1 : 0;

                if (vi < 0 || vi >= (int)vertices.size())
                    continue;

                vBuffer.push_back(vertices[vi].x);
                vBuffer.push_back(vertices[vi].y);
                vBuffer.push_back(vertices[vi].z);

                if (ti >= 0 && ti < (int)texCoords.size())
                {
                    vBuffer.push_back(texCoords[ti].s);
                    vBuffer.push_back(texCoords[ti].t);
                }
                else
                {
                    vBuffer.push_back(0.0f);
                    vBuffer.push_back(0.0f);
                }

                if (ni >= 0 && ni < (int)normals.size())
                {
                    vBuffer.push_back(normals[ni].x);
                    vBuffer.push_back(normals[ni].y);
                    vBuffer.push_back(normals[ni].z);
                }
                else
                {
                    vBuffer.push_back(0.0f);
                    vBuffer.push_back(0.0f);
                    vBuffer.push_back(1.0f);
                }
            }
        }
    }

    file.close();

    if (vBuffer.empty())
    {
        std::cerr << "Nenhum vértice carregado do OBJ: " << filePath << std::endl;
        nVertices = 0;
        return 0;
    }

    GLuint VBO, VAO;
    glGenBuffers(1, &VBO);
    glGenVertexArrays(1, &VAO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vBuffer.size() * sizeof(GLfloat), vBuffer.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(5 * sizeof(GLfloat)));
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    nVertices = static_cast<int>(vBuffer.size() / 8);
    return VAO;
}

GLuint loadTexture(const std::string& filePath)
{
    GLuint texID;
    glGenTextures(1, &texID);
    glBindTexture(GL_TEXTURE_2D, texID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int width, height, nrChannels;
    unsigned char* data = stbi_load(filePath.c_str(), &width, &height, &nrChannels, 0);

    if (data)
    {
        if (nrChannels == 3)
        {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        }
        else
        {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        }
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture " << filePath << std::endl;
    }

    stbi_image_free(data);
    glBindTexture(GL_TEXTURE_2D, 0);
    return texID;
}

int setupShader()
{
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    GLint success;
    GLchar infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cerr << "Vertex shader compilation failed:\n" << infoLog << std::endl;
    }

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cerr << "Fragment shader compilation failed:\n" << infoLog << std::endl;
    }

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cerr << "Shader program linking failed:\n" << infoLog << std::endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return static_cast<int>(shaderProgram);
}
