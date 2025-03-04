#include <stdio.h>
#include <math.h>

#include <ew/external/glad.h>
#include <ew/shader.h>
#include <ew/model.h>
#include <ew/camera.h>
#include <ew/transform.h>
#include <ew/cameraController.h>
#include <ew/texture.h>
#include <ew/procGen.h>

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "Classes/Animator.h"
#include "Classes/AnimationClip.h"

#include <iostream>

struct Material {
	float Ka = 1.0;
	float Kd = 0.5;
	float Ks = 0.5;
	float Shininess = 128;
};
Material material;

struct Light {
	
	glm::vec3 direction;

	float ambient;
	float diffuse;
	float specular;
};
Light directionalLight;




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

// Post Processing
unsigned int framebuffer;
unsigned int textureColorbuffer;
unsigned int rbo;
unsigned int vao;

// Shadow Mapping
unsigned int depthMapFBO;
const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
unsigned int depthMap;

unsigned int theDepthShaderToQuery;
float bias = 0.1;

// Animation
ec::Animator animator;
ec::AnimationClip theClip;


int main() {
	GLFWwindow* window = initWindow("Assignment 0", screenWidth, screenHeight);

	// Shaders
	ew::Shader shader = ew::Shader("assets/lit.vert", "assets/lit.frag");
	ew::Shader postProcessShaderNormal = ew::Shader("assets/buffer.vert", "assets/buffer.frag");
	ew::Shader postProcessInvertedShader = ew::Shader("assets/buffer.vert", "assets/invertedBuffer.frag");
	ew::Shader postProcessBlurShader = ew::Shader("assets/buffer.vert", "assets/blurBuffer.frag");
	ew::Shader simpleDepthShader = ew::Shader("assets/simpleDepthShader.vert", "assets/simpleDepthShader.frag");
	ew::Shader drawingDepthBufferShader = ew::Shader("assets/buffer.vert", "assets/drawingDepthShader.frag"); // Unused debug shader
	ew::Shader shadowRenderShader = ew::Shader("assets/shadowRender.vert", "assets/shadowRender.frag");

	// Model and Meshes
	ew::Model monkeyModel = ew::Model("assets/suzanne.obj");
	ew::Mesh planeMesh = ew::createPlane(5,5,5);

	// Textures
	GLuint brickTexture = ew::loadTexture("assets/brick_color.jpg");

	// Declaring variables
	ew::Transform monkeyTransform;
	ew::Transform planeTransform;
	glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

	//Initializing Variables
	planeTransform.position = glm::vec3(0, -2, 0);

	// Lighting
	directionalLight.ambient = 0.5f;
	directionalLight.diffuse = 0.5f;
	directionalLight.specular = 0.5f;
	directionalLight.direction = glm::vec3(0.5, 1, 0.5);

	// Animating
	theClip.duration = 1.0f;

	theClip.positionKeys[0].timestamp = 0.0f;
	theClip.rotationKeys[0].timestamp = 0.0f;
	theClip.scaleKeys[0].timestamp = 0.0f;
	theClip.positionKeys[0].keyFrame = glm::vec3(0.0f, 5.0f, 0.0f);
	theClip.rotationKeys[0].keyFrame = glm::vec3(0.0f, 90.0f, 0.0f);
	theClip.scaleKeys[0].keyFrame = glm::vec3(1.0f, 1.0f, 1.0f);

	theClip.positionKeys[1].timestamp = 0.5f;
	theClip.rotationKeys[1].timestamp = 0.5f;
	theClip.scaleKeys[1].timestamp = 0.5f;
	theClip.positionKeys[1].keyFrame = glm::vec3(5.0f, 5.0f, 0.0f);
	theClip.rotationKeys[1].keyFrame = glm::vec3(90.0f, 90.0f, 0.0f);
	theClip.scaleKeys[1].keyFrame = glm::vec3(1.0f, 2.0f, 1.0f);

	theClip.positionKeys[2].timestamp = 1.0f;
	theClip.rotationKeys[2].timestamp = 1.0f;
	theClip.scaleKeys[2].timestamp = 1.0f;
	theClip.positionKeys[2].keyFrame = glm::vec3(5.0f, 5.0f, 5.0f);
	theClip.rotationKeys[2].keyFrame = glm::vec3(90.0f, 90.0f, -90.0f);
	theClip.scaleKeys[2].keyFrame = glm::vec3(2.0f, 2.0f, 1.0f);
	animator.clip = theClip;
	animator.isPlaying = false;
	animator.isLooping = false;
	animator.playbackTime = 0.0f;
	animator.playbackSpeed = 0.001f;


	// Post Processing
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

	// SHADOWMAPPING
	glGenFramebuffers(1, &depthMapFBO);

	// Depth buffer
	glGenTextures(1, &depthMap);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
		SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// Binding
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	
	// Resetting
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, brickTexture);
	
	
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
		glm::vec3 lightDirection = normalize(-directionalLight.direction);

		// Position Updates
		cameraController.move(window, &camera, deltaTime);
		monkeyTransform = animator.calcNewFrame();

		glm::vec3 lightPosition = -glm::vec3(lightDirection.x * 3,lightDirection.y * 3, lightDirection.z * 3);

		// SHADOW PASS
		float near_plane = 0.01f, far_plane = 10.0f;
		glm::mat4 lightProjection = glm::ortho(-3.0f, 3.0f, -3.0f, 3.0f, near_plane, far_plane);
		glm::mat4 lightView = glm::lookAt(glm::vec3(lightPosition),
			glm::vec3(0.0f, 0.0f, 0.0f),
			glm::vec3(0.0f, 1.0f, 0.0f));
		glm::mat4 lightSpaceMatrix = lightProjection * lightView;

		// TODO :: Finish up here with the shadow pass

		glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
		glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
		glClear(GL_DEPTH_BUFFER_BIT);
		glCullFace(GL_FRONT);

		simpleDepthShader.use();
		simpleDepthShader.setMat4("_lightSpaceMatrix", lightSpaceMatrix);

		simpleDepthShader.setMat4("_Model", monkeyTransform.modelMatrix());
		monkeyModel.draw(); //Draws monkey model using current shader
		simpleDepthShader.setMat4("_Model", planeTransform.modelMatrix());
		planeMesh.draw(); // Drawing the plane

		glCullFace(GL_BACK);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		   

		// RENDER PASS
		glViewport(0, 0, screenWidth, screenHeight);
		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
		glClearColor(0.6f, 0.8f, 0.92f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		// Lighting
		
		/*{
			// Shader
			shader.use();
			shader.setInt("_MainTex", 0);
			shader.setVec3("_EyePos", camera.position);
			shader.setFloat("_Material.Ka", material.Ka);
			shader.setFloat("_Material.Kd", material.Kd);
			shader.setFloat("_Material.Ks", material.Ks);
			shader.setFloat("_Material.Shininess", material.Shininess);
			shader.setVec3("_LightDirection", lightDirection);

			// Models
			glBindTexture(GL_TEXTURE_2D, brickTexture);
			monkeyTransform.rotation = glm::rotate(monkeyTransform.rotation, deltaTime, glm::vec3(0.0, 1.0, 0.0));
			shader.setMat4("_Model", monkeyTransform.modelMatrix());
			shader.setMat4("_ViewProjection", camera.projectionMatrix() * camera.viewMatrix());
			monkeyModel.draw(); //Draws monkey model using current shader

			shader.setMat4("_Model", planeTransform.modelMatrix());
			shader.setMat4("_ViewProjection", camera.projectionMatrix() * camera.viewMatrix());
			planeMesh.draw(); // Drawing the plane
		}*/
		// ^ Old render code

		
		shadowRenderShader.use();
		shadowRenderShader.setMat4("projection", camera.projectionMatrix());
		shadowRenderShader.setMat4("view", camera.viewMatrix());
		shadowRenderShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
		shadowRenderShader.setVec3("lightPos", lightPosition);
		shadowRenderShader.setVec3("lightPos", camera.position);
		shadowRenderShader.setFloat("bias", bias);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, brickTexture);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, depthMap);
		shadowRenderShader.setInt("diffuseTexture", 0);
		shadowRenderShader.setInt("shadowMap", 1);

		

		// Models
		shadowRenderShader.setMat4("model", monkeyTransform.modelMatrix());
		monkeyModel.draw(); //Draws monkey model using current shader
		shadowRenderShader.setMat4("model", planeTransform.modelMatrix());
		planeMesh.draw(); // Drawing the plane


		glfwSwapBuffers(window);

		// POST PROCESS PASS
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
		//std::cout << directionalLight.direction.x << ", " << directionalLight.direction.y << ", " << directionalLight.direction.z << std::endl;
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
	if (ImGui::CollapsingHeader("Light Controls")) {
		if (ImGui::CollapsingHeader("Light Direction")) {
			ImGui::SliderFloat("X component", &directionalLight.direction.x, -1.0f, 1.0f);
			ImGui::SliderFloat("Y component", &directionalLight.direction.y, -1.0f, 1.0f);
			ImGui::SliderFloat("Z component", &directionalLight.direction.z, -1.0f, 1.0f);
		}
		/*if (ImGui::CollapsingHeader("Light Data")) {
			ImGui::SliderFloat("Ambient", &directionalLight.ambient, 0.0f, 1.0f);
			ImGui::SliderFloat("Diffuse", &directionalLight.diffuse, 0.0f, 1.0f);
			ImGui::SliderFloat("Specular", &directionalLight.specular, 0.0f, 1.0f);
		}*/
	}
	ImGui::SliderFloat("Bias", &bias, 0.0f, 1.0f);
	

	if (ImGui::CollapsingHeader("Animation Controls")) {
		
		ImGui::Checkbox("Animating", &animator.isPlaying);
		ImGui::Checkbox("Loop", &animator.isLooping);
		ImGui::DragFloat("Playback Speed", &animator.playbackSpeed, 0.0001f);
		ImGui::DragFloat("Playback Duration", &animator.clip.duration, 0.1f);
		ImGui::SliderFloat("Playback Time", &animator.playbackTime, 0.0001f, animator.clip.duration);
		
		if (ImGui::CollapsingHeader("Position Keyframes")) {
			ImGui::SliderFloat("Timestamp PosFrame1", &animator.clip.positionKeys[0].timestamp, 0.001f, animator.clip.duration);
			ImGui::DragFloat3("Value PosFrame1", &animator.clip.positionKeys[0].keyFrame.x, 0.1f);
			ImGui::SliderFloat("Timestamp PosFrame2", &animator.clip.positionKeys[1].timestamp, 0.001f, animator.clip.duration);
			ImGui::DragFloat3("Value PosFrame2", &animator.clip.positionKeys[1].keyFrame.x, 0.1f);
			ImGui::SliderFloat("Timestamp PosFrame3", &animator.clip.positionKeys[2].timestamp, 0.001f, animator.clip.duration);
			ImGui::DragFloat3("Value PosFrame3", &animator.clip.positionKeys[2].keyFrame.x, 0.1f);
		}
		if (ImGui::CollapsingHeader("Rotation Keyframes")) {
			ImGui::SliderFloat("Timestamp RotFrame1", &animator.clip.rotationKeys[0].timestamp, 0.001f, animator.clip.duration);
			ImGui::DragFloat3("Value RotFrame1", &animator.clip.rotationKeys[0].keyFrame.x, 0.1f);
			ImGui::SliderFloat("Timestamp RotFrame2", &animator.clip.rotationKeys[1].timestamp, 0.001f, animator.clip.duration);
			ImGui::DragFloat3("Value RotFrame2", &animator.clip.rotationKeys[1].keyFrame.x, 0.1f);
			ImGui::SliderFloat("Timestamp RotFrame3", &animator.clip.rotationKeys[2].timestamp, 0.001f, animator.clip.duration);
			ImGui::DragFloat3("Value RotFrame3", &animator.clip.rotationKeys[2].keyFrame.x, 0.1f);
		}
		if (ImGui::CollapsingHeader("Scale Keyframes")) {
			ImGui::SliderFloat("Timestamp ScaFrame1", &animator.clip.scaleKeys[0].timestamp, 0.001f, animator.clip.duration);
			ImGui::DragFloat3("Value ScaFrame1", &animator.clip.scaleKeys[0].keyFrame.x, 0.1f);
			ImGui::SliderFloat("Timestamp ScaFrame2", &animator.clip.scaleKeys[1].timestamp, 0.001f, animator.clip.duration);
			ImGui::DragFloat3("Value ScaFrame2", &animator.clip.scaleKeys[1].keyFrame.x, 0.1f);
			ImGui::SliderFloat("Timestamp ScaFrame3", &animator.clip.scaleKeys[2].timestamp, 0.001f, animator.clip.duration);
			ImGui::DragFloat3("Value ScaFrame3", &animator.clip.scaleKeys[2].keyFrame.x, 0.1f);
		}
	}


	ImGui::Text("Add Controls Here!");
	ImGui::End();

	ImGui::Begin("Shadow Map");
	//Using a Child allow to fill all the space of the window.
	ImGui::BeginChild("Shadow Map");
	//Stretch image to be window size
	ImVec2 windowSize = ImGui::GetWindowSize();
	//Invert 0-1 V to flip vertically for ImGui display
	//shadowMap is the texture2D handle
	ImGui::Image((ImTextureID)depthMap, windowSize, ImVec2(0, 1), ImVec2(1, 0));
	ImGui::EndChild();
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




