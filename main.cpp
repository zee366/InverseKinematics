#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "MatrixMath.h"
#include "stb_image.h"
#include "Shader.h"
#include "Camera.h"

#include <iostream>
#include <vector>

using namespace mm; // MatrixMath.h

// Forward declaration of callbacks and utility functions
// ------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void mouse_button_callback(GLFWwindow*, int button, int action, int mods);
void processInput(GLFWwindow* window);
unsigned int loadTexture(const char* path);
void calcVerticesFromAngles(float vertices[], float A, float B, float C, float& theta, float& phi, float& psi);

// Screen resolution
// -----------------
const unsigned int SCR_WIDTH = 1600;
const unsigned int SCR_HEIGHT = 1600;

// camera
// ------
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;

// timing
// ------
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// misc
// ----
const int ATTRIBUTES_PER_VERTEX = 3;
const float EPSILON = 0.03f;
glm::vec3 target = glm::vec3(0.0f);
float mouseToScrnMultiplier = 2.0f * camera.Position.z * glm::tan(glm::radians(camera.Zoom / 2.0f));
bool moveLine = false;

int main() {
	// glfw: initialize and configure
	// ------------------------------
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // uncomment this statement to fix compilation on OS X
#endif

	// glfw window creation
	// --------------------
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Comp477 A2 - Part 2", NULL, NULL);
	if (window == NULL) {
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}

	// Set window context and register callback functions
	// --------------------------------------------------
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);

	// tell GLFW to capture our mouse
	// ------------------------------
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

	// glad: load all OpenGL function pointers
	// ---------------------------------------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	// build and compile our shader program
	// ------------------------------------
	Shader lineShader("lineStripV.glsl", "lineStripF.glsl"); // Add path to vertex and fragment shaders here

	// set up vertex data (and buffer(s)) and configure vertex attributes
	// ------------------------------------------------------------------
	
	// segment lengths
	float A = 0.5f;
	float B = 0.25f;
	float C = 0.25f;

	// angles
	float theta = 0.0f;
	float phi = QUARTER_PI / 2.0f;
	float psi = QUARTER_PI;

	// arm
	float vertices[12];
	calcVerticesFromAngles(vertices, A, B, C, theta, phi, psi);

	// circle
	std::vector<glm::vec3> circleVertices;
	float radius = A + B + C;
	for (float i = 0.0f; i < TWO_PI; i += 0.05f) {
		float x = radius * glm::cos(i);
		float y = radius * glm::sin(i);
		float z = 0.0f;
		circleVertices.push_back(glm::vec3(x, y, z));
	}

	// arm
	unsigned int VBO, VAO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	// circle
	unsigned int circleVBO, circleVAO;
	glGenVertexArrays(1, &circleVAO);
	glGenBuffers(1, &circleVBO);

	glBindVertexArray(circleVAO);
	glBindBuffer(GL_ARRAY_BUFFER, circleVBO);
	glBufferData(GL_ARRAY_BUFFER, circleVertices.size() * sizeof(glm::vec3), &circleVertices.front(), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	// unbind
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	

	// Set OpenGL render modes
	// -----------------------
	glLineWidth(4.0f);

	// render loop
	// -----------
	while (!glfwWindowShouldClose(window)) {
		// per-frame time logic
		// --------------------
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// input
		// -----
		processInput(window);

		// move the line if a target is set
		if (moveLine) {
			Jacobian3x2 j(A, B, C, theta, phi, psi);
			glm::mat2x3 jPlus = calcJPlus(j.mat);

			glm::vec3 endPoint = glm::vec3(vertices[9], vertices[10], vertices[11]);
			glm::vec3 direction = target - endPoint;

			glm::vec2 deltaDirection = glm::vec2(0.02f * direction.x, 0.02f * direction.y);
			glm::vec3 deltaAngles = jPlus * deltaDirection;

			theta += deltaAngles.x;
			phi += deltaAngles.y;
			psi += deltaAngles.z;
			calcVerticesFromAngles(vertices, A, B, C, theta, phi, psi);

			glBindBuffer(GL_ARRAY_BUFFER, VBO);
			glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

			if (abs(direction.x) < EPSILON && abs(direction.y) < EPSILON)
				moveLine = false;
		}

		// render
		// ------
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		// View and Projection transformations
		// -----------------------------------
		// view/projection transformations
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		glm::mat4 view = camera.GetViewMatrix();

		// Bind textures here
		// ------------------

		// Choose a shader, bind the VAO that will use this shader, call draw functions and finally unbind
		// -----------------------------------------------------------------------------------------------
		lineShader.use();

		// set uniforms here
		// -----------------
		lineShader.setMat4("projection", projection);
		lineShader.setMat4("view", view);

		// bind
		// ----
		glBindVertexArray(VAO);

		// call glDraw functions here
		// --------------------------
		glDrawArrays(GL_LINE_STRIP, 0, (sizeof(vertices) / sizeof(float)) / ATTRIBUTES_PER_VERTEX);

		glBindVertexArray(circleVAO);
		glDrawArrays(GL_LINE_LOOP, 0, circleVertices.size());

		// unbind
		// ------
		glBindVertexArray(0);

		// Repeat the above sequence for other VAOs or different shaders
		// -------------------------------------------------------------

		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		// -------------------------------------------------------------------------------
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// optional: de-allocate all resources once they've outlived their purpose:
	// ------------------------------------------------------------------------
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);

	// glfw: terminate, clearing all previously allocated GLFW resources.
	// ------------------------------------------------------------------
	glfwTerminate();
	return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
}

// utility function for loading a 2D texture from file
// ---------------------------------------------------
unsigned int loadTexture(char const* path) {
	unsigned int textureID;
	glGenTextures(1, &textureID);

	int width, height, nrComponents;
	unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
	if (data) {
		GLenum format;
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else {
		std::cout << "Texture failed to load at path: " << path << std::endl;
		stbi_image_free(data);
	}

	return textureID;
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	// make sure the viewport matches the new dimensions; note that width and
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
	lastX = xpos;
	lastY = ypos;
}

// glfw: whenever a mouse button is pressed, this callback is called
// -----------------------------------------------------------------
void mouse_button_callback(GLFWwindow*, int button, int action, int mods) {
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
		target.x = mouseToScrnMultiplier * ((lastX - (SCR_WIDTH / 2)) / SCR_WIDTH);
		target.y = mouseToScrnMultiplier * (((SCR_HEIGHT / 2) - lastY) / SCR_HEIGHT);

		std::cout << "New target position at: " << target.x << " " << target.y << " " << std::endl;
		moveLine = true;
	}
}

// calculate positions of the vertices based on line segment lengths and the given angles
// --------------------------------------------------------------------------------------
void calcVerticesFromAngles(float vertices[], float A, float B, float C, float& theta, float& phi, float& psi) {
	float cosTheta = glm::cos(theta);
	float cosThetaPhi = glm::cos(theta + phi);
	float cosThetaPhiPsi = glm::cos(theta + phi + psi);

	float sinTheta = glm::sin(theta);
	float sinThetaPhi = glm::sin(theta + phi);
	float sinThetaPhiPsi = glm::sin(theta + phi + psi);

	vertices[0] = 0.0f;
	vertices[1] = 0.0f;
	vertices[2] = 0.0f;

	vertices[3] = A * cosTheta;
	vertices[4] = A * sinTheta;
	vertices[5] = 0.0f;

	vertices[6] = A * cosTheta + B * cosThetaPhi;
	vertices[7] = A * sinTheta + B * sinThetaPhi;
	vertices[8] = 0.0f;

	vertices[9] = A * cosTheta + B * cosThetaPhi + C * cosThetaPhiPsi;
	vertices[10] = A * sinTheta + B * sinThetaPhi + C * sinThetaPhiPsi;
	vertices[11] = 0.0f;
}