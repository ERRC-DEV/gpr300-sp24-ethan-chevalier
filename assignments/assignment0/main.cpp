#include <stdio.h>
#include <math.h>

#include <ew/external/glad.h>
#include <ew/shader.h>
#include <ew/model.h>
#include <ew/camera.h>
#include <ew/transform.h>
#include <ew/cameraController.h>
#include <ew/texture.h>

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <iostream>

struct Material {
	float Ka = 1.0;
	float Kd = 0.5;
	float Ks = 0.5;
	float Shininess = 128;
}material;


void framebufferSizeCallback(GLFWwindow* window, int width, int height);
GLFWwindow* initWindow(const char* title, int width, int height);
void drawUI();

//Global state
int screenWidth = 1080;
int screenHeight = 720;
float prevFrameTime;
float deltaTime;
ew::Camera camera;
ew::CameraController cameraController;
int shaderToUse = 0;

unsigned int framebuffer;
unsigned int textureColorbuffer;
unsigned int rbo;
unsigned int vao;


int main() {
	GLFWwindow* window = initWindow("Assignment 0", screenWidth, screenHeight);
	ew::Shader shader = ew::Shader("assets/lit.vert", "assets/lit.frag");
	ew::Shader postProcessShaderNormal = ew::Shader("assets/buffer.vert", "assets/buffer.frag");
	ew::Shader postProcessInvertedShader = ew::Shader("assets/buffer.vert", "assets/invertedBuffer.frag");
	ew::Shader postProcessBlurShader = ew::Shader("assets/buffer.vert", "assets/blurBuffer.frag");
	ew::Model monkeyModel = ew::Model("assets/suzanne.obj");
	GLuint brickTexture = ew::loadTexture("assets/brick_color.jpg");
	ew::Transform monkeyTransform;
	glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

	glGenFramebuffers(1, &framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

	glGenTextures(1, &textureColorbuffer);
	glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, screenWidth, screenHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0); // This line may need to be moved after the binding

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorbuffer, 0); // Attach texture to the framebuffer

	glGenRenderbuffers(1, &rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, screenWidth, screenHeight);
	glBindRenderbuffer(GL_RENDERBUFFER, 0); // This line may need to be moved after the binding

	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo); // attach rbo to framebuffer

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glCreateVertexArrays(1, &vao);

	
	

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, brickTexture);
	
	shader.use();
	shader.setInt("_MainTex", 0);
	shader.setVec3("_EyePos", camera.position);
	shader.setFloat("_Material.Ka", material.Ka);
	shader.setFloat("_Material.Kd", material.Kd);
	shader.setFloat("_Material.Ks", material.Ks);
	shader.setFloat("_Material.Shininess", material.Shininess);
	
	camera.position = glm::vec3(0.0f, 0.0f, 5.0f);
	camera.target = glm::vec3(0.0f, 0.0f, 0.0f); //Look at the center of the scene
	camera.aspectRatio = (float)screenWidth / screenHeight;
	camera.fov = 60.0f; //Vertical field of view, in degrees

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK); //Back face culling
	glEnable(GL_DEPTH_TEST); //Depth testing


	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();

		float time = (float)glfwGetTime();
		deltaTime = time - prevFrameTime;
		prevFrameTime = time;

		//RENDER
		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
		glClearColor(0.6f, 0.8f, 0.92f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		shader.use();
		glBindTexture(GL_TEXTURE_2D, brickTexture);
		monkeyTransform.rotation = glm::rotate(monkeyTransform.rotation, deltaTime, glm::vec3(0.0, 1.0, 0.0));
		shader.setMat4("_Model", monkeyTransform.modelMatrix());
		cameraController.move(window, &camera, deltaTime);
		shader.setMat4("_ViewProjection", camera.projectionMatrix() * camera.viewMatrix());
		monkeyModel.draw(); //Draws monkey model using current shader

		

		glfwSwapBuffers(window);


		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		switch (shaderToUse)
		{
			case 0:
				postProcessShaderNormal.use();
				break;
			case 1:
				postProcessInvertedShader.use();
				break;
			case 2:
				postProcessBlurShader.use();
				break;
		}
		
		glBindTextureUnit(0, textureColorbuffer);
		glBindVertexArray(vao);
		glDrawArrays(GL_TRIANGLES, 0, 6); //6 for quad, 3 for triangle

		drawUI();
	}
	printf("Shutting down...");
}
void resetCamera(ew::Camera* camera, ew::CameraController* controller) {
	camera->position = glm::vec3(0, 0, 5.0f);
	camera->target = glm::vec3(0);
	controller->yaw = controller->pitch = 0;
}

void drawUI() {
	ImGui_ImplGlfw_NewFrame();
	ImGui_ImplOpenGL3_NewFrame();
	ImGui::NewFrame();

	ImGui::Begin("Settings");
	if (ImGui::Button("Reset Camera")) {
		resetCamera(&camera, &cameraController);
	}
	if (ImGui::CollapsingHeader("Material")) {
		ImGui::SliderFloat("AmbientK", &material.Ka, 0.0f, 1.0f);
		ImGui::SliderFloat("DiffuseK", &material.Kd, 0.0f, 1.0f);
		ImGui::SliderFloat("SpecularK", &material.Ks, 0.0f, 1.0f);
		ImGui::SliderFloat("Shininess", &material.Shininess, 2.0f, 1024.0f);
	}
	if (ImGui::CollapsingHeader("Shaders")) {
		if (ImGui::Button("Normal Shader"))
		{
			shaderToUse = 0;
		}
		if (ImGui::Button("Inverted Shader"))
		{
			shaderToUse = 1;
		}
		if (ImGui::Button("Blur Shader"))
		{
			shaderToUse = 2;
		}
	}

	ImGui::Text("Add Controls Here!");
	ImGui::End();

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void framebufferSizeCallback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
	screenWidth = width;
	screenHeight = height;
}

/// <summary>
/// Initializes GLFW, GLAD, and IMGUI
/// </summary>
/// <param name="title">Window title</param>
/// <param name="width">Window width</param>
/// <param name="height">Window height</param>
/// <returns>Returns window handle on success or null on fail</returns>
GLFWwindow* initWindow(const char* title, int width, int height) {
	printf("Initializing...");
	if (!glfwInit()) {
		printf("GLFW failed to init!");
		return nullptr;
	}

	GLFWwindow* window = glfwCreateWindow(width, height, title, NULL, NULL);
	if (window == NULL) {
		printf("GLFW failed to create window");
		return nullptr;
	}
	glfwMakeContextCurrent(window);

	if (!gladLoadGL(glfwGetProcAddress)) {
		printf("GLAD Failed to load GL headers");
		return nullptr;
	}

	//Initialize ImGUI
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init();

	return window;
}




