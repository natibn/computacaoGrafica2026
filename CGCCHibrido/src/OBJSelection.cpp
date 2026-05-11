#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

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

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
int setupShader();
int loadSimpleOBJ(const std::string& filePath, int& nVertices, std::string& textureName);
GLuint loadTexture(const std::string& filePath);

const GLchar* vertexShaderSource = "#version 450\n"
"layout(location = 0) in vec3 position;\n"
"layout(location = 1) in vec2 texCoord;\n"
"uniform mat4 model;\n"
"out vec2 TexCoord;\n"
"void main()\n"
"{\n"
"    gl_Position = model * vec4(position, 1.0);\n"
"    TexCoord = texCoord;\n"
"}\0";

const GLchar* fragmentShaderSource = "#version 450\n"
"in vec2 TexCoord;\n"
"uniform sampler2D textureSampler;\n"
"uniform vec4 objectColor;\n"
"uniform bool useTexture;\n"
"out vec4 color;\n"
"void main()\n"
"{\n"
"    if (useTexture)\n"
"        color = texture(textureSampler, TexCoord);\n"
"    else\n"
"        color = objectColor;\n"
"}\0";

int main()
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

    std::cout << "========================================" << std::endl;
    std::cout << "OBJ Selection Example" << std::endl;
    std::cout << "Controles:" << std::endl;
    std::cout << "  TAB: Selecionar próximo objeto" << std::endl;
    std::cout << "  1, 2, 3: Selecionar objeto direto" << std::endl;
    std::cout << "  W/S: Mover objeto selecionado para frente/trás" << std::endl;
    std::cout << "  A/D: Mover objeto selecionado para esquerda/direita" << std::endl;
    std::cout << "  Q/E: Mover objeto selecionado para cima/baixo" << std::endl;
    std::cout << "  K/L: Diminuir/Aumentar escala do objeto selecionado" << std::endl;
    std::cout << "  X/Y/Z: Selecionar eixo de rotação" << std::endl;
    std::cout << "  R/T: Girar objeto selecionado (+/-15 graus no eixo)" << std::endl;
    std::cout << "  ESC: Sair" << std::endl;
    std::cout << "========================================" << std::endl;

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);

    GLuint shaderProgram = setupShader();
    glUseProgram(shaderProgram);

    std::vector<std::string> files = {"../assets/Modelos3D/Cube.obj", "../assets/Modelos3D/Suzanne.obj", "../assets/Modelos3D/SuzanneSubdiv1.obj"};
    std::vector<glm::vec3> positions = { glm::vec3(-2.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(2.0f, 0.0f, 0.0f) };
    std::vector<glm::vec3> baseColors = { glm::vec3(0.7f, 0.5f, 0.3f), glm::vec3(0.5f, 0.8f, 0.5f), glm::vec3(0.6f, 0.6f, 0.9f) };

    for (size_t i = 0; i < files.size(); i++)
    {
        int nVertices;
        std::string textureName;
        GLuint vao = loadSimpleOBJ(files[i], nVertices, textureName);
        if (vao == 0 || nVertices <= 0)
        {
            std::cerr << "Falha ao carregar OBJ: " << files[i] << std::endl;
            continue;
        }

        MeshData mesh;
        mesh.VAO = vao;
        mesh.vertexCount = nVertices;
        mesh.name = files[i].substr(files[i].find_last_of("/\\") + 1);
        mesh.position = positions[i];
        mesh.scale = glm::vec3(0.6f);
        mesh.color = baseColors[i];
        mesh.rotation = glm::vec3(0.0f, 0.0f, 0.0f);
        mesh.textureID = 0;
        mesh.hasTexture = false;
        mesh.textureName = textureName;

        if (!textureName.empty())
        {
            std::string texturePath = files[i].substr(0, files[i].find_last_of("/\\") + 1) + textureName;
            mesh.textureID = loadTexture(texturePath);
            if (mesh.textureID != 0)
            {
                mesh.hasTexture = true;
                std::cout << "Textura carregada: " << texturePath << std::endl;
            }
            else
            {
                std::cerr << "Falha ao carregar textura: " << texturePath << std::endl;
            }
        }

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

    if (textureLoc != -1)
    {
        glUniform1i(textureLoc, 0);
    }

    glEnable(GL_DEPTH_TEST);

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

            glBindVertexArray(mesh.VAO);
            glDrawArrays(GL_TRIANGLES, 0, mesh.vertexCount);
        }

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

    if (key == GLFW_KEY_1 && action == GLFW_PRESS && meshes.size() >= 1) selectedMeshIndex = 0;
    if (key == GLFW_KEY_2 && action == GLFW_PRESS && meshes.size() >= 2) selectedMeshIndex = 1;
    if (key == GLFW_KEY_3 && action == GLFW_PRESS && meshes.size() >= 3) selectedMeshIndex = 2;

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

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    nVertices = static_cast<int>(vBuffer.size() / 5);
    return VAO;
}

GLuint loadTexture(const std::string& filePath)
{
    int width, height, nrChannels;
    unsigned char* data = stbi_load(filePath.c_str(), &width, &height, &nrChannels, 0);
    if (!data)
    {
        std::cerr << "Falha ao carregar imagem: " << filePath << std::endl;
        return 0;
    }

    GLuint texID;
    glGenTextures(1, &texID);
    glBindTexture(GL_TEXTURE_2D, texID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    if (nrChannels == 3)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    }
    else if (nrChannels == 4)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    }
    else
    {
        std::cerr << "Formato de imagem não suportado: " << filePath << std::endl;
        stbi_image_free(data);
        glBindTexture(GL_TEXTURE_2D, 0);
        return 0;
    }

    glGenerateMipmap(GL_TEXTURE_2D);
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
