#include <iostream>
#include <vector>
#include <string>

#define GLEW_STATIC
#include <GL\glew.h>
#include <GLFW\glfw3.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "Shader.h"
#include "Camera.h"

// Global constants
const GLint WINDOW_WIDTH = 1600;
const GLint WINDOW_HEIGHT = 900;
const GLchar* WINDOW_TITLE = "Phong Demo";

const GLchar* LIGHTING_VERTEX_SHADER = "lighting.vert";
const GLchar* LIGHTING_FRAGMENT_SHADER = "lighting.frag";

const GLchar* LAMP_VERTEX_SHADER = "lamp.vert";
const GLchar* LAMP_FRAGMENT_SHADER = "lamp.frag";

const GLchar* CONTAINER_FILE_NAME = "container2.png";
const GLchar* CONTAINER_SPECULAR_FILE_NAME = "container2_specular.png";

// Function prototypes
void framebufferSizeCallback(GLFWwindow* window, GLint width, GLint height);
void cursorPosCallback(GLFWwindow* window, GLdouble xpos, GLdouble ypos);
void mouseScrollCallback(GLFWwindow* window, GLdouble xoffset, GLdouble yoffset);
void runRenderLoop(GLFWwindow* window, Shader& lightingShader, Shader& lampShader);
GLFWwindow* createWindow();
void configureGLFW();
void processInput(GLFWwindow* window);
std::vector<GLfloat> genCubeVertices();
GLuint loadTexture(const GLchar* filepath);

// Global variables
Camera cam;
GLfloat lastFrameTime = 0.0f;
GLfloat lastXPos = WINDOW_WIDTH / 2.0f;
GLfloat lastYPos = WINDOW_HEIGHT / 2.0f;
GLuint containerTex;
GLuint containerSpecularTex;
glm::vec3 lampPos;

int main()
{
	// Initialize GLFW and create a window
	configureGLFW();
	GLFWwindow* window = createWindow();

	// Check if window was successfully created
	if (window == nullptr)
	{
		std::cout << "ERROR: failed to create GLFW window" << std::endl;
		glfwTerminate();
		return EXIT_FAILURE;
	}
	
	// Initialize GLEW to fetch OpenGL function pointers
	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
	if (err != GLEW_OK)
	{
		std::cout << "ERROR: failed to initialize GLEW\n" << glewGetErrorString(err) << std::endl;
		return EXIT_FAILURE;
	}

	// Set initial viewport dimensions
	glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// Set callbacks
	glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
	glfwSetCursorPosCallback(window, cursorPosCallback);
	glfwSetScrollCallback(window, mouseScrollCallback);

	// Define the vertices of a cube
	std::vector<GLfloat> vertexVector(genCubeVertices());

	// Create a vertex array object and bind it to the OpenGL context
	GLuint VAO;
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	// Create a vertex buffer object and bind it to the ARRAY_BUFFER target
	GLuint VBO;
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	// Buffer vertices to GPU
	glBufferData(GL_ARRAY_BUFFER, vertexVector.size() * sizeof(GLfloat), vertexVector.data(), GL_STATIC_DRAW);

	// Configure vertex attributes
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(6 * sizeof(GLfloat)));
	glEnableVertexAttribArray(2);

	// Create shaders
	Shader lightingShader(LIGHTING_VERTEX_SHADER, LIGHTING_FRAGMENT_SHADER);
	Shader lampShader(LAMP_VERTEX_SHADER, LAMP_FRAGMENT_SHADER);

	// Load textures
	containerTex = loadTexture(CONTAINER_FILE_NAME);
	containerSpecularTex = loadTexture(CONTAINER_SPECULAR_FILE_NAME);

	// Loop until GLFW window is told to close
	lastFrameTime = (GLfloat)glfwGetTime();
	glfwSetCursorPos(window, WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT / 2.0f);
	runRenderLoop(window, lightingShader, lampShader);

	// Destroy instance of GLFW and exit program
	glfwTerminate();
	return EXIT_SUCCESS;
}

void runRenderLoop(GLFWwindow* window, Shader& lightingShader, Shader& lampShader)
{
	glm::vec3 cubePositions[] = {
		glm::vec3(0.0f,  0.0f,  0.0f),
		glm::vec3(2.0f,  5.0f, -15.0f),
		glm::vec3(-1.5f, -2.2f, -2.5f),
		glm::vec3(-3.8f, -2.0f, -12.3f),
		glm::vec3(2.4f, -0.4f, -3.5f),
		glm::vec3(-1.7f,  3.0f, -7.5f),
		glm::vec3(1.3f, -2.0f, -2.5f),
		glm::vec3(1.5f,  2.0f, -2.5f),
		glm::vec3(1.5f,  0.2f, -1.5f),
		glm::vec3(-1.3f,  1.0f, -1.5f)
	};

	glm::vec3 pointLightPositions[] = {
		glm::vec3(0.7f,  0.2f,  2.0f),
		glm::vec3(2.3f, -3.3f, -4.0f),
		glm::vec3(-4.0f,  2.0f, -12.0f),
		glm::vec3(0.0f,  0.0f, -3.0f)
	};

	glEnable(GL_DEPTH_TEST);

	lampPos = glm::vec3(1.0f, 2.0f, 0.0f);

	// Set light vectors
	lightingShader.use();
	lightingShader.setVec3("light.position", lampPos);
	lightingShader.setVec3("light.ambient", 0.2f, 0.2f, 0.2f);
	lightingShader.setVec3("light.diffuse", 0.5f, 0.5f, 0.5f);
	lightingShader.setVec3("light.specular", 1.0f, 1.0f, 1.0f);
	lightingShader.setFloat("light.constant", 1.0f);
	lightingShader.setFloat("light.linear", 0.09f);
	lightingShader.setFloat("light.quadratic", 0.032f);

	// Define materials
	Material containerMat;
	containerMat.diffuse = 0; // Texture unit 0
	containerMat.specular = 1; // Texture unit 1
	containerMat.shininess = 0.5 * 128;

	lightingShader.setMaterial("material", containerMat);

	// directional light
	lightingShader.setVec3("dirLight.direction", -0.2f, -1.0f, -0.3f);
	lightingShader.setVec3("dirLight.ambient", 0.05f, 0.05f, 0.05f);
	lightingShader.setVec3("dirLight.diffuse", 0.4f, 0.4f, 0.4f);
	lightingShader.setVec3("dirLight.specular", 0.5f, 0.5f, 0.5f);
	// point light 1
	lightingShader.setVec3("pointLights[0].position", pointLightPositions[0]);
	lightingShader.setVec3("pointLights[0].ambient", 0.05f, 0.05f, 0.05f);
	lightingShader.setVec3("pointLights[0].diffuse", 0.8f, 0.8f, 0.8f);
	lightingShader.setVec3("pointLights[0].specular", 1.0f, 1.0f, 1.0f);
	lightingShader.setFloat("pointLights[0].constant", 1.0f);
	lightingShader.setFloat("pointLights[0].linear", 0.09f);
	lightingShader.setFloat("pointLights[0].quadratic", 0.032f);
	// point light 2
	lightingShader.setVec3("pointLights[1].position", pointLightPositions[1]);
	lightingShader.setVec3("pointLights[1].ambient", 0.05f, 0.05f, 0.05f);
	lightingShader.setVec3("pointLights[1].diffuse", 0.8f, 0.8f, 0.8f);
	lightingShader.setVec3("pointLights[1].specular", 1.0f, 1.0f, 1.0f);
	lightingShader.setFloat("pointLights[1].constant", 1.0f);
	lightingShader.setFloat("pointLights[1].linear", 0.09f);
	lightingShader.setFloat("pointLights[1].quadratic", 0.032f);
	// point light 3
	lightingShader.setVec3("pointLights[2].position", pointLightPositions[2]);
	lightingShader.setVec3("pointLights[2].ambient", 0.05f, 0.05f, 0.05f);
	lightingShader.setVec3("pointLights[2].diffuse", 0.8f, 0.8f, 0.8f);
	lightingShader.setVec3("pointLights[2].specular", 1.0f, 1.0f, 1.0f);
	lightingShader.setFloat("pointLights[2].constant", 1.0f);
	lightingShader.setFloat("pointLights[2].linear", 0.09f);
	lightingShader.setFloat("pointLights[2].quadratic", 0.032f);
	// point light 4
	lightingShader.setVec3("pointLights[3].position", pointLightPositions[3]);
	lightingShader.setVec3("pointLights[3].ambient", 0.05f, 0.05f, 0.05f);
	lightingShader.setVec3("pointLights[3].diffuse", 0.8f, 0.8f, 0.8f);
	lightingShader.setVec3("pointLights[3].specular", 1.0f, 1.0f, 1.0f);
	lightingShader.setFloat("pointLights[3].constant", 1.0f);
	lightingShader.setFloat("pointLights[3].linear", 0.09f);
	lightingShader.setFloat("pointLights[3].quadratic", 0.032f);

	while (!glfwWindowShouldClose(window))
	{
		processInput(window);

		// Clear contents of window
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glm::mat4 view;
		view = cam.getViewMatrix();

		glm::mat4 projection;
		projection = cam.getProjectionMatrix((GLfloat)WINDOW_WIDTH / WINDOW_HEIGHT, 0.1f, 100.0f);

		// Prepare lighting shader and cube for rendering
		lightingShader.use();
		// Set lighting shader uniforms
		lightingShader.setMat4("view", view);
		lightingShader.setMat4("projection", projection);
		lightingShader.setVec3("viewPos", cam.getPosition());

		// Draw cube

		// Bind container diffuse map to texture unit 0
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, containerTex);
		// Bind container specular map to texture unit 1
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, containerSpecularTex);

		// Draw container
		lightingShader.use();
		for (GLuint i = 0; i < 10; ++i)
		{
			glm::mat4 cubeModel;
			cubeModel = glm::translate(cubeModel, cubePositions[i]);
			GLfloat angle = 20.0f * i;
			cubeModel = glm::rotate(cubeModel, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
			lightingShader.setMat4("model", cubeModel);
			glDrawArrays(GL_TRIANGLES, 0, 36);
		}

		for (GLuint i = 0; i < 4; ++i)
		{
			glm::mat4 lampModel;
			lampModel = glm::translate(lampModel, pointLightPositions[i]);
			lampModel = glm::scale(lampModel, glm::vec3(0.2f, 0.2f, 0.2f));
			lampShader.use();
			lampShader.setMat4("model", lampModel);
			lampShader.setMat4("view", view);
			lampShader.setMat4("projection", projection);
			glDrawArrays(GL_TRIANGLES, 0, 36);
		}

		// Swap framebuffers and poll for events (keyboard/mouse input, window resizing, etc.)
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
}

void processInput(GLFWwindow* window)
{
	GLfloat currentTime = (GLfloat)glfwGetTime();
	GLfloat deltaT = currentTime - lastFrameTime;
	lastFrameTime = currentTime;

	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, GLFW_TRUE);
	}
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
	{
		cam.processKeyboard(FORWARD, deltaT);
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
	{
		cam.processKeyboard(BACKWARD, deltaT);
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
	{
		cam.processKeyboard(LEFT, deltaT);
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
	{
		cam.processKeyboard(RIGHT, deltaT);
	}
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
	{
		cam.processKeyboard(UP, deltaT);
	}
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
	{
		cam.processKeyboard(DOWN, deltaT);
	}
	if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS)
	{
		cam.processKeyboard(FOCUS, deltaT);
	}
}

GLFWwindow* createWindow()
{
	GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE, nullptr, nullptr);
	glfwMakeContextCurrent(window);
	return window;
}

void configureGLFW()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
}

std::vector<GLfloat> genCubeVertices()
{
	std::vector<GLfloat> vertexVector = {
		// positions          // normals           // texture coords
		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
		0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f,
		0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
		0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
		-0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,

		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 0.0f,
		0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 0.0f,
		0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 1.0f,
		0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 1.0f,
		-0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 0.0f,

		-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
		-0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
		-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
		-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

		0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
		0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
		0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
		0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
		0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
		0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,
		0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f,
		0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
		0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,

		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,
		0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f,
		0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
		0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
		-0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f,
		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f
	};
	
	return vertexVector;
}

void cursorPosCallback(GLFWwindow* window, GLdouble xpos, GLdouble ypos)
{
	GLfloat xoffset = (GLfloat)(xpos - lastXPos);
	GLfloat yoffset = (GLfloat)(ypos - lastYPos);
	lastXPos = (GLfloat)xpos;
	lastYPos = (GLfloat)ypos;

	cam.processMouseMove(xoffset, yoffset);
}

void mouseScrollCallback(GLFWwindow* window, GLdouble xoffset, GLdouble yoffset)
{
	cam.processMouseScroll((GLfloat)yoffset);
}

void framebufferSizeCallback(GLFWwindow* window, GLint width, GLint height)
{
	glViewport(0, 0, width, height);
}

GLuint loadTexture(const GLchar* filepath)
{
	GLuint id;
	// Generate (globally declared) texture object and bind it to OpenGL's 2D texture target
	glGenTextures(1, &id);
	glBindTexture(GL_TEXTURE_2D, id);
	// Configure texture parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// Load image
	GLint width, height, numChannels;
	GLubyte* data = stbi_load(filepath, &width, &height, &numChannels, 0);
	if (data)
	{
		// Set format based on number of color channels in image
		GLenum format;
		if (numChannels == 3)
		{
			format = GL_RGB;
		}
		else if (numChannels == 4)
		{
			format = GL_RGBA;
		}
		else
		{
			std::cout << "ERROR LOADING TEXTURE: Not a valid 3 or 4 component image format" << std::endl;
		}

		// Load image data into container texture and generate its mipmap
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		// Image failed to load correctly
		std::cout << "Failed to load texture:\n" << stbi_failure_reason() << std::endl;
	}
	// Free container image data
	stbi_image_free(data);

	return id;
}