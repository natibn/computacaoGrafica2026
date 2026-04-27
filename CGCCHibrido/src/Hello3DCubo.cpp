#include <iostream>
#include <string>
#include <assert.h>

using namespace std;

// GLAD
#include <glad/glad.h>

// GLFW
#include <GLFW/glfw3.h>

//GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


// Protótipo da função de callback de teclado
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);

// Protótipos das funções
int setupShader();
int setupGeometry();

// Dimensões da janela (pode ser alterado em tempo de execução)
const GLuint WIDTH = 1000, HEIGHT = 1000;

// Código fonte do Vertex Shader (em GLSL): ainda hardcoded
const GLchar* vertexShaderSource = "#version 450\n"
"layout (location = 0) in vec3 position;\n"
"layout (location = 1) in vec3 color;\n"
"uniform mat4 model;\n"
"out vec4 finalColor;\n"
"void main()\n"
"{\n"
//...pode ter mais linhas de código aqui!
"gl_Position = model * vec4(position, 1.0);\n"
"finalColor = vec4(color, 1.0);\n"
"}\0";

//Código fonte do Fragment Shader (em GLSL): ainda hardcoded
const GLchar* fragmentShaderSource = "#version 450\n"
"in vec4 finalColor;\n"
"out vec4 color;\n"
"void main()\n"
"{\n"
"color = finalColor;\n"
"}\n\0";

bool rotateX=false, rotateY=false, rotateZ=false;

// Variáveis para translação
bool moveForward=false, moveBackward=false, moveLeft=false, moveRight=false, moveUp=false, moveDown=false;

// Variáveis para escala
bool scaleUp=false, scaleDown=false;

// Índice do cubo selecionado (0-5)
int cuboSelecionado = 0;

// Estrutura para representar um cubo
struct Cubo {
    glm::vec3 posicao;
    glm::vec3 escala;
    glm::vec3 rotacao;  // eixo de rotação
    float anguloRotacao;
    glm::vec3 cor;
};

// Vetor de cubos instanciados
std::vector<Cubo> cubos;

// Inicializar cubos na cena
void inicializarCubos()
{
    // Cubo 1 - centro
    cubos.push_back({glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.5f), glm::vec3(1.0f, 0.0f, 0.0f), 0.0f, glm::vec3(1.0f, 0.0f, 0.0f)});
    
    // Cubo 2 - à esquerda
    cubos.push_back({glm::vec3(-1.5f, 0.0f, 0.0f), glm::vec3(0.4f), glm::vec3(0.0f, 1.0f, 0.0f), 0.0f, glm::vec3(0.0f, 1.0f, 0.0f)});
    
    // Cubo 3 - à direita
    cubos.push_back({glm::vec3(1.5f, 0.0f, 0.0f), glm::vec3(0.4f), glm::vec3(0.0f, 0.0f, 1.0f), 0.0f, glm::vec3(0.0f, 0.0f, 1.0f)});
    
    // Cubo 4 - acima
    cubos.push_back({glm::vec3(0.0f, 1.5f, 0.0f), glm::vec3(0.35f), glm::vec3(1.0f, 1.0f, 0.0f), 0.0f, glm::vec3(1.0f, 1.0f, 0.0f)});
    
    // Cubo 5 - abaixo
    cubos.push_back({glm::vec3(0.0f, -1.5f, 0.0f), glm::vec3(0.35f), glm::vec3(0.0f, 1.0f, 1.0f), 0.0f, glm::vec3(0.5f, 0.0f, 1.0f)});
    
    // Cubo 6 - mais afastado
    cubos.push_back({glm::vec3(0.0f, 0.0f, -2.0f), glm::vec3(0.6f), glm::vec3(0.0f, 0.0f, 1.0f), 0.0f, glm::vec3(1.0f, 1.0f, 1.0f)});
}

// Função MAIN
int main()
{
	// Inicialização da GLFW
	glfwInit();

	//Muita atenção aqui: alguns ambientes não aceitam essas configurações
	//Você deve adaptar para a versão do OpenGL suportada por sua placa
	//Sugestão: comente essas linhas de código para desobrir a versão e
	//depois atualize (por exemplo: 4.5 com 4 e 5)
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	//Essencial para computadores da Apple
//#ifdef __APPLE__
//	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
//#endif

	// Criação da janela GLFW
	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Cubo 3D -- Natália!", nullptr, nullptr);
	glfwMakeContextCurrent(window);

	// Fazendo o registro da função de callback para a janela GLFW
	glfwSetKeyCallback(window, key_callback);

	// GLAD: carrega todos os ponteiros d funções da OpenGL
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;

	}

	// Obtendo as informações de versão
	const GLubyte* renderer = glGetString(GL_RENDERER); /* get renderer string */
	const GLubyte* version = glGetString(GL_VERSION); /* version as a string */
	cout << "Renderer: " << renderer << endl;
	cout << "OpenGL version supported " << version << endl;

	// Definindo as dimensões da viewport com as mesmas dimensões da janela da aplicação
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	glViewport(0, 0, width, height);


	// Compilando e buildando o programa de shader
	GLuint shaderID = setupShader();

	// Gerando um buffer simples, com a geometria de um triângulo
	GLuint VAO = setupGeometry();

	// Inicializar cubos
	inicializarCubos();

	glUseProgram(shaderID);

	glm::mat4 model = glm::mat4(1); //matriz identidade;
	GLint modelLoc = glGetUniformLocation(shaderID, "model");
	// Rotação inicial
	model = glm::rotate(model, /*(GLfloat)glfwGetTime()*/glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	glEnable(GL_DEPTH_TEST);


	// Loop da aplicação - "game loop"
	while (!glfwWindowShouldClose(window))
	{
		// Checa se houveram eventos de input (key pressed, mouse moved etc.) e chama as funções de callback correspondentes
		glfwPollEvents();

		// Limpa o buffer de cor
		glClearColor(0.2f, 0.2f, 0.2f, 1.0f); //cor de fundo cinza escuro
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glLineWidth(10);
		glPointSize(20);

		float angle = (GLfloat)glfwGetTime();

		// Atualizar posição baseada nos controles de translação (cubo selecionado)
		if (moveForward) cubos[cuboSelecionado].posicao.z += 0.01f;
		if (moveBackward) cubos[cuboSelecionado].posicao.z -= 0.01f;
		if (moveLeft) cubos[cuboSelecionado].posicao.x -= 0.01f;
		if (moveRight) cubos[cuboSelecionado].posicao.x += 0.01f;
		if (moveUp) cubos[cuboSelecionado].posicao.y += 0.01f;
		if (moveDown) cubos[cuboSelecionado].posicao.y -= 0.01f;

		// Limites para evitar que o cubo saia da visão
		cubos[cuboSelecionado].posicao.x = glm::clamp(cubos[cuboSelecionado].posicao.x, -3.0f, 3.0f);
		cubos[cuboSelecionado].posicao.y = glm::clamp(cubos[cuboSelecionado].posicao.y, -3.0f, 3.0f);
		cubos[cuboSelecionado].posicao.z = glm::clamp(cubos[cuboSelecionado].posicao.z, -3.0f, 3.0f);

		// Atualizar escala (cubo selecionado)
		if (scaleUp) cubos[cuboSelecionado].escala.x += 0.01f;
		if (scaleDown) cubos[cuboSelecionado].escala.x -= 0.01f;
		cubos[cuboSelecionado].escala.x = glm::max(0.1f, cubos[cuboSelecionado].escala.x);
		cubos[cuboSelecionado].escala.y = cubos[cuboSelecionado].escala.x;
		cubos[cuboSelecionado].escala.z = cubos[cuboSelecionado].escala.x;

		// Renderizar cada cubo
		glBindVertexArray(VAO);
		
		for (size_t i = 0; i < cubos.size(); i++)
		{
			Cubo& c = cubos[i];
			
			// Atualizar ângulo de rotação de cada cubo
			c.anguloRotacao = angle * (0.5f + i * 0.3f); // cada cubo gira em velocidade diferente
			
			model = glm::mat4(1);
			// Aplicar transformações na ordem: escala, rotação, translação
			model = glm::scale(model, c.escala);
			model = glm::rotate(model, c.anguloRotacao, c.rotacao);
			model = glm::translate(model, c.posicao);

			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
			
			// Desenhar cubo
			glDrawArrays(GL_TRIANGLES, 0, 36);
		}

		glBindVertexArray(0);

		// Troca os buffers da tela
		glfwSwapBuffers(window);
	}
	// Pede pra OpenGL desalocar os buffers
	glDeleteVertexArrays(1, &VAO);
	// Finaliza a execução da GLFW, limpando os recursos alocados por ela
	glfwTerminate();
	return 0;
}

// Função de callback de teclado - só pode ter uma instância (deve ser estática se
// estiver dentro de uma classe) - É chamada sempre que uma tecla for pressionada
// ou solta via GLFW
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

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

	// Seleção de cubo com teclas numéricas 1-6
	if (key == GLFW_KEY_1 && action == GLFW_PRESS) cuboSelecionado = 0;
	if (key == GLFW_KEY_2 && action == GLFW_PRESS) cuboSelecionado = 1;
	if (key == GLFW_KEY_3 && action == GLFW_PRESS) cuboSelecionado = 2;
	if (key == GLFW_KEY_4 && action == GLFW_PRESS) cuboSelecionado = 3;
	if (key == GLFW_KEY_5 && action == GLFW_PRESS) cuboSelecionado = 4;
	if (key == GLFW_KEY_6 && action == GLFW_PRESS) cuboSelecionado = 5;

	// Controles de translação
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

	if (key == GLFW_KEY_I)
	{
		if (action == GLFW_PRESS) moveUp = true;
		else if (action == GLFW_RELEASE) moveUp = false;
	}

	if (key == GLFW_KEY_J)
	{
		if (action == GLFW_PRESS) moveDown = true;
		else if (action == GLFW_RELEASE) moveDown = false;
	}

	// Controles de escala
	if (key == GLFW_KEY_LEFT_BRACKET && action == GLFW_PRESS) // [
	{
		scaleDown = true;
		scaleUp = false;
	}

	if (key == GLFW_KEY_RIGHT_BRACKET && action == GLFW_PRESS) // ]
	{
		scaleUp = true;
		scaleDown = false;
	}

	if ((key == GLFW_KEY_LEFT_BRACKET || key == GLFW_KEY_RIGHT_BRACKET) && action == GLFW_RELEASE)
	{
		scaleUp = false;
		scaleDown = false;
	}
}

//Esta função está basntante hardcoded - objetivo é compilar e "buildar" um programa de
// shader simples e único neste exemplo de código
// O código fonte do vertex e fragment shader está nos arrays vertexShaderSource e
// fragmentShader source no iniçio deste arquivo
// A função retorna o identificador do programa de shader
int setupShader()
{
	// Vertex shader
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);
	// Checando erros de compilação (exibição via log no terminal)
	GLint success;
	GLchar infoLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
	}
	// Fragment shader
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);
	// Checando erros de compilação (exibição via log no terminal)
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
	}
	// Linkando os shaders e criando o identificador do programa de shader
	GLuint shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);
	// Checando por erros de linkagem
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
	}
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	return shaderProgram;
}

// Esta função está bastante harcoded - objetivo é criar os buffers que armazenam a 
// geometria de um triângulo
// Apenas atributo coordenada nos vértices
// 1 VBO com as coordenadas, VAO com apenas 1 ponteiro para atributo
// A função retorna o identificador do VAO
int setupGeometry()
{
	// Aqui setamos as coordenadas x, y e z do triângulo e as armazenamos de forma
	// sequencial, já visando mandar para o VBO (Vertex Buffer Objects)
	// Cada atributo do vértice (coordenada, cores, coordenadas de textura, normal, etc)
	// Pode ser arazenado em um VBO único ou em VBOs separados
	GLfloat vertices[] = {
		// Cubo: 6 faces, cada uma com 2 triângulos (12 triângulos, 36 vértices)
		// Formato: x, y, z, r, g, b

		// Face frontal (z = 0.5): E, F, G, H
		-0.5, -0.5,  0.5, 1.0, 0.0, 0.0, // E (vermelho)
		 0.5, -0.5,  0.5, 0.0, 1.0, 0.0, // F (verde)
		 0.5,  0.5,  0.5, 0.0, 0.0, 1.0, // G (azul)

		-0.5, -0.5,  0.5, 1.0, 0.0, 0.0, // E
		 0.5,  0.5,  0.5, 0.0, 0.0, 1.0, // G
		-0.5,  0.5,  0.5, 1.0, 1.0, 0.0, // H (amarelo)

		// Face traseira (z = -0.5): A, B, C, D
		-0.5, -0.5, -0.5, 1.0, 0.5, 0.0, // A (laranja)
		 0.5, -0.5, -0.5, 0.5, 1.0, 0.0, // B (verde claro)
		 0.5,  0.5, -0.5, 0.0, 1.0, 0.5, // C (ciano)

		-0.5, -0.5, -0.5, 1.0, 0.5, 0.0, // A
		 0.5,  0.5, -0.5, 0.0, 1.0, 0.5, // C
		-0.5,  0.5, -0.5, 0.5, 0.0, 1.0, // D (roxo)

		// Face esquerda (x = -0.5): A, D, H, E
		-0.5, -0.5, -0.5, 1.0, 0.5, 0.0, // A
		-0.5,  0.5, -0.5, 0.5, 0.0, 1.0, // D
		-0.5,  0.5,  0.5, 1.0, 1.0, 0.0, // H

		-0.5, -0.5, -0.5, 1.0, 0.5, 0.0, // A
		-0.5,  0.5,  0.5, 1.0, 1.0, 0.0, // H
		-0.5, -0.5,  0.5, 1.0, 0.0, 0.0, // E

		// Face direita (x = 0.5): B, F, G, C
		 0.5, -0.5, -0.5, 0.5, 1.0, 0.0, // B
		 0.5, -0.5,  0.5, 0.0, 1.0, 0.0, // F
		 0.5,  0.5,  0.5, 0.0, 0.0, 1.0, // G

		 0.5, -0.5, -0.5, 0.5, 1.0, 0.0, // B
		 0.5,  0.5,  0.5, 0.0, 0.0, 1.0, // G
		 0.5,  0.5, -0.5, 0.0, 1.0, 0.5, // C

		// Face superior (y = 0.5): D, C, G, H
		-0.5,  0.5, -0.5, 0.5, 0.0, 1.0, // D
		 0.5,  0.5, -0.5, 0.0, 1.0, 0.5, // C
		 0.5,  0.5,  0.5, 0.0, 0.0, 1.0, // G

		-0.5,  0.5, -0.5, 0.5, 0.0, 1.0, // D
		 0.5,  0.5,  0.5, 0.0, 0.0, 1.0, // G
		-0.5,  0.5,  0.5, 1.0, 1.0, 0.0, // H

		// Face inferior (y = -0.5): A, E, F, B
		-0.5, -0.5, -0.5, 1.0, 0.5, 0.0, // A
		-0.5, -0.5,  0.5, 1.0, 0.0, 0.0, // E
		 0.5, -0.5,  0.5, 0.0, 1.0, 0.0, // F

		-0.5, -0.5, -0.5, 1.0, 0.5, 0.0, // A
		 0.5, -0.5,  0.5, 0.0, 1.0, 0.0, // F
		 0.5, -0.5, -0.5, 0.5, 1.0, 0.0, // B
	};

	GLuint VBO, VAO;

	//Geração do identificador do VBO
	glGenBuffers(1, &VBO);

	//Faz a conexão (vincula) do buffer como um buffer de array
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	//Envia os dados do array de floats para o buffer da OpenGl
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	//Geração do identificador do VAO (Vertex Array Object)
	glGenVertexArrays(1, &VAO);

	// Vincula (bind) o VAO primeiro, e em seguida  conecta e seta o(s) buffer(s) de vértices
	// e os ponteiros para os atributos 
	glBindVertexArray(VAO);
	
	//Para cada atributo do vertice, criamos um "AttribPointer" (ponteiro para o atributo), indicando: 
	// Localização no shader * (a localização dos atributos devem ser correspondentes no layout especificado no vertex shader)
	// Numero de valores que o atributo tem (por ex, 3 coordenadas xyz) 
	// Tipo do dado
	// Se está normalizado (entre zero e um)
	// Tamanho em bytes 
	// Deslocamento a partir do byte zero 
	
	//Atributo posição (x, y, z)
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);

	//Atributo cor (r, g, b)
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3*sizeof(GLfloat)));
	glEnableVertexAttribArray(1);


	// Observe que isso é permitido, a chamada para glVertexAttribPointer registrou o VBO como o objeto de buffer de vértice 
	// atualmente vinculado - para que depois possamos desvincular com segurança
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Desvincula o VAO (é uma boa prática desvincular qualquer buffer ou array para evitar bugs medonhos)
	glBindVertexArray(0);

	return VAO;
}

