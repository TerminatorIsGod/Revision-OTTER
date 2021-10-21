#include <Logging.h>
#include <iostream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <filesystem>
#include <json.hpp>
#include <fstream>

#include <GLM/glm.hpp>
#include <GLM/gtc/matrix_transform.hpp>
#include <GLM/gtc/type_ptr.hpp>

#include "IndexBuffer.h"
#include "VertexBuffer.h"
#include "VertexArrayObject.h"
#include "Shader.h"
#include "Camera.h"

#include "Utils/MeshBuilder.h"
#include "Utils/MeshFactory.h"
#include "Utils/ObjLoader.h"
#include "VertexTypes.h"

#include "bullet/btBulletDynamicsCommon.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "Player.h"
#include "DynamicObject.h"

#define LOG_GL_NOTIFICATIONS

Camera::Sptr m_camera;

static size_t OBJSelected = 0;

bool allowMouse = false;

float dt;

//Cleaning & saving/loading
std::vector<Player> o_players;
std::vector<DynamicObject> o_dynamicObjects;
//


/*
	Handles debug messages from OpenGL
	https://www.khronos.org/opengl/wiki/Debug_Output#Message_Components
	@param source    Which part of OpenGL dispatched the message
	@param type      The type of message (ex: error, performance issues, deprecated behavior)
	@param id        The ID of the error or message (to distinguish between different types of errors, like nullref or index out of range)
	@param severity  The severity of the message (from High to Notification)
	@param length    The length of the message
	@param message   The human readable message from OpenGL
	@param userParam The pointer we set with glDebugMessageCallback (should be the game pointer)
*/
void GlDebugMessage(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam) {
	std::string sourceTxt;
	switch (source) {
		case GL_DEBUG_SOURCE_API: sourceTxt = "DEBUG"; break;
		case GL_DEBUG_SOURCE_WINDOW_SYSTEM: sourceTxt = "WINDOW"; break;
		case GL_DEBUG_SOURCE_SHADER_COMPILER: sourceTxt = "SHADER"; break;
		case GL_DEBUG_SOURCE_THIRD_PARTY: sourceTxt = "THIRD PARTY"; break;
		case GL_DEBUG_SOURCE_APPLICATION: sourceTxt = "APP"; break;
		case GL_DEBUG_SOURCE_OTHER: default: sourceTxt = "OTHER"; break;
	}
	switch (severity) {
		case GL_DEBUG_SEVERITY_LOW:          LOG_INFO("[{}] {}", sourceTxt, message); break;
		case GL_DEBUG_SEVERITY_MEDIUM:       LOG_WARN("[{}] {}", sourceTxt, message); break;
		case GL_DEBUG_SEVERITY_HIGH:         LOG_ERROR("[{}] {}", sourceTxt, message); break;
			#ifdef LOG_GL_NOTIFICATIONS
		case GL_DEBUG_SEVERITY_NOTIFICATION: LOG_INFO("[{}] {}", sourceTxt, message); break;
			#endif
		default: break;
	}
}

// Stores our GLFW window in a global variable for now
GLFWwindow* window;
// The current size of our window in pixels
glm::ivec2 windowSize = glm::ivec2(800, 800);
// The title of our GLFW window
std::string windowTitle = "Nathan Tyborski - 100781410";

void GlfwWindowResizedCallback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
	windowSize = glm::ivec2(width, height);
}

/// <summary>
/// Handles intializing GLFW, should be called before initGLAD, but after Logger::Init()
/// Also handles creating the GLFW window
/// </summary>
/// <returns>True if GLFW was initialized, false if otherwise</returns>
bool initGLFW() {
	// Initialize GLFW
	if (glfwInit() == GLFW_FALSE) {
		LOG_ERROR("Failed to initialize GLFW");
		return false;
	}

	//Create a new GLFW window and make it current
	window = glfwCreateWindow(windowSize.x, windowSize.y, windowTitle.c_str(), nullptr, nullptr);
	glfwMakeContextCurrent(window);
	
	// Set our window resized callback
	glfwSetWindowSizeCallback(window, GlfwWindowResizedCallback);

	return true;
}

/// <summary>
/// Handles initializing GLAD and preparing our GLFW window for OpenGL calls
/// </summary>
/// <returns>True if GLAD is loaded, false if there was an error</returns>
bool initGLAD() {
	if (gladLoadGLLoader((GLADloadproc)glfwGetProcAddress) == 0) {
		LOG_ERROR("Failed to initialize Glad");
		return false;
	}
	return true;
}

//float lastx, lasty;

// position
glm::vec3 position = glm::vec3(0, 0, 5);
// horizontal angle : toward -Z
float horizontalAngle = 3.14f;
// vertical angle : 0, look at the horizon
float verticalAngle = 0.0f;
// Initial Field of View
float initialFoV = 45.0f;

float speed = 0.0025f; // 3 units / second

float mouseSpeed = 0.00025f;

glm::vec3 right;
glm::vec3 up;
glm::vec3 cdir;

Player* m_player = nullptr;
glm::vec3 m_cameraPos = glm::vec3(0, 3, 3);
glm::vec3 m_cameraDir = glm::vec3(0.0f, 0.0f, 1.0f);
glm::vec3 m_cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
glm::vec3 m_payerPos = glm::vec3(0, 3, 3);

void playerMovement() {

	float velocityValue = 0.25 * dt;
	// Move forward
	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
		//OBJBody[1]->applyCentralImpulse(btVector3(cdir.x, cdir.y, cdir.z));
		m_player->SetVelocity(velocityValue * cdir);
		//position += cdir * dt * speed;
	}
	// Move backward
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
		m_player->SetVelocity((-1) * velocityValue * cdir);
		//position -= cdir * dt * speed;
	}
	// Strafe right
	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
		m_player->SetVelocity(velocityValue * glm::normalize(glm::cross(cdir, m_cameraUp)));
		//position += right * dt * speed;
	}
	// Strafe left
	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
		m_player->SetVelocity((-1) * velocityValue * glm::normalize(glm::cross(cdir, m_cameraUp)));
		//position -= right * dt * speed;
	}

	std::cout << "Camera Dir: " << cdir.x << " " << cdir.y << " " << cdir.z << std::endl;

	//glm::vec3 getPlayerPos = m_player->GetPosition();
	//std::cout << "Player X: " << getPlayerPos.x << " Player Y: " << getPlayerPos.y << " Player Z: " << getPlayerPos.z << std::endl;

	//m_camera->SetPosition(position);

	//std::cout << "Position X: " << position.x << " Position Y: " << position.y << " Position Z: " << position.z << std::endl;


}

static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{

	if (allowMouse)
		return;

	//mouse stuff - gives x/y pos depending on location within window

	float x, y;

	float centerx = (windowSize.x / 2);
	float centery = (windowSize.y / 2);

	x = ((xpos - centerx) / centerx);
	y = (-(ypos - centery) / centery);

	//std::cout << "Mouse X: " << x << " Mouse Y: " << y << std::endl;


	glfwSetCursorPos(window, centerx, centery);

	horizontalAngle += mouseSpeed * dt * float(centerx - xpos);
	verticalAngle += mouseSpeed * dt * float(centery - ypos);

	//std::cout << "HAngle: " << horizontalAngle << " VAngle: " << verticalAngle << std::endl;

	glm::vec3 direction(
		cos(verticalAngle) * sin(horizontalAngle),
		sin(verticalAngle),
		cos(verticalAngle) * cos(horizontalAngle)
	);

	cdir = direction;

	// Right vector
	right = glm::vec3(
		sin(horizontalAngle - 3.14f / 2.0f),
		0,
		cos(horizontalAngle - 3.14f / 2.0f)
	);

	// Up vector : perpendicular to both direction and right
	up = glm::cross(right, direction);

	glm::lookAt(
		position,             // Camera is here
		position + direction, // and looks here : at the same position, plus "direction"
		up                    // Head is up (set to 0,-1,0 to look upside-down)
	);


	m_cameraDir = direction;
	m_cameraUp = up;

	//m_camera->SetForward(direction);
	//m_camera->SetUp(up);
	//m_camera->SetPosition(position);
	//m_camera->SetFovDegrees(initialFoV);

}

int main() {

	Logger::Init(); // We'll borrow the logger from the toolkit, but we need to initialize it

	//Initialize GLFW
	if (!initGLFW())
		return 1;

	//Initialize GLAD
	if (!initGLAD())
		return 1;


	btDefaultCollisionConfiguration* m_CollisionConfig = new btDefaultCollisionConfiguration();
	btCollisionDispatcher* m_CollisionDispatcher = new btCollisionDispatcher(m_CollisionConfig);
	btBroadphaseInterface* m_OverlappingPairCache = new btDbvtBroadphase();
	btSequentialImpulseConstraintSolver* m_Solver = new btSequentialImpulseConstraintSolver();
	btDiscreteDynamicsWorld* m_DynamicsWorld = new btDiscreteDynamicsWorld(m_CollisionDispatcher, m_OverlappingPairCache, m_Solver, m_CollisionConfig);

	m_DynamicsWorld->setGravity(btVector3(0, -4, 0));


	// Let OpenGL know that we want debug output, and route it to our handler function
	glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	glDebugMessageCallback(GlDebugMessage, nullptr);


	// Load our shaders
	Shader* shader = new Shader();
	shader->LoadShaderPartFromFile("shaders/vertex_shader.glsl", ShaderPartType::Vertex);
	shader->LoadShaderPartFromFile("shaders/frag_shader.glsl", ShaderPartType::Fragment);
	shader->Link();

	// GL states, we'll enable depth testing and backface fulling
	glEnable(GL_DEPTH_TEST);
	// glEnable(GL_CULL_FACE);
	//glCullFace(GL_BACK);
	glClearColor(0.2f, 0.2f, 0.2f, 1.0f);

	// Get uniform location for the model view projection
	m_camera = Camera::Create();
	m_camera->SetPosition(glm::vec3(0, 3, 3));
	m_camera->LookAt(glm::vec3(0.0f));

	// camera pos,	doesn't matter currently, size, camera object, bullet physics world, object file, nickanme to help identify
	m_player = new Player(m_camera->GetPosition(), m_payerPos, glm::vec3(50, 1, 1), m_camera, m_DynamicsWorld, ObjLoader::LoadFromFile("blank.obj"), "Player"); //create player
	m_player->LoadShaderFiles("shaders/vertex_shader.glsl", "shaders/frag_shader.glsl"); //Loads vertex shader then loads Fragment shader
	

	// Our high-precision timer
	double lastFrame = glfwGetTime();

	LOG_INFO("Starting mesh build");

	MeshBuilder<VertexPosCol> mesh;
	MeshFactory::AddIcoSphere(mesh, glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.5f), 3);
	MeshFactory::AddCube(mesh, glm::vec3(0.0f), glm::vec3(0.5f));
	VertexArrayObject::Sptr vao3 = mesh.Bake();


	std::vector<VertexArrayObject::Sptr> vaos;

	std::vector<glm::vec3> OBJposition;

	DynamicObject* o_object_3 = new DynamicObject(glm::vec3(0,10,0), glm::vec3(1,1,1), btScalar(1.0f), m_DynamicsWorld, ObjLoader::LoadFromFile("Monkey.obj"), "DynamicObject1");
	o_object_3->LoadShaderFiles("shaders/vertex_shader.glsl", "shaders/frag_shader.glsl"); //Loads vertex shader then loads Fragment shader

	//DynamicObject* o_object_1 = new DynamicObject(glm::vec3(0, 0, 0), glm::vec3(1, 1, 1), btScalar(0.0f), m_DynamicsWorld, ObjLoader::LoadFromFile("Monkey.obj"), "StaticObject1");
	//o_object_1->LoadShaderFiles("shaders/vertex_shader.glsl", "shaders/frag_shader.glsl"); //Loads vertex shader then loads Fragment shader

	//DynamicObject* o_object_2 = new DynamicObject(glm::vec3(0, -10, 0), glm::vec3(100, 1, 100), btScalar(0.0f), m_DynamicsWorld, ObjLoader::LoadFromFile("flat100.obj"), "StaticObject1");
	//o_object_2->LoadShaderFiles("shaders/vertex_shader.glsl", "shaders/frag_shader.glsl"); //Loads vertex shader then loads Fragment shader

	DynamicObject* o_object_4 = new DynamicObject(glm::vec3(0, -10, 0), glm::vec3(100, 1, 100), btScalar(0.0f), m_DynamicsWorld, ObjLoader::LoadFromFile("map.obj"), "StaticObject1");
	o_object_4->LoadShaderFiles("shaders/vertex_shader.glsl", "shaders/frag_shader.glsl"); //Loads vertex shader then loads Fragment shader



	/*vaos.push_back(ObjLoader::LoadFromFile("Monkey.obj")); //camera
	addCollider(vaos.size() - 1, btVector3(1, 1, 1), btScalar(1));
	transforms.push_back(glm::translate(glm::mat4(1.0f), glm::vec3(2, 1, 0)));

	vaos.push_back(ObjLoader::LoadFromFile("Monkey.obj"));
	addCollider(vaos.size() - 1, btVector3(1, 1, 1), btScalar(1));
	transforms.push_back(glm::translate(glm::mat4(1.0f), glm::vec3(2, 1, 0)));

	
	//transforms[1] = glm::rotate(glm::mat4(1.0f), static_cast<float>(glm::radians(90)), glm::vec3(1, 0, 0)) + glm::translate(glm::mat4(1.0f), glm::vec3(2,1,0)); //this lets you do rotation and transformation

	vaos.push_back(ObjLoader::LoadFromFile("Monkey.obj"));
	addCollider(vaos.size() - 1, btVector3(1, 1, 1), btScalar(1));
	transforms.push_back(glm::translate(glm::mat4(1.0f), glm::vec3(2, 1, 0)));

	vaos.push_back(ObjLoader::LoadFromFile("Monkey.obj"));
	addCollider(vaos.size() - 1, btVector3(1, 1, 1), btScalar(1));
	transforms.push_back(glm::translate(glm::mat4(1.0f), glm::vec3(2, 1, 0)));*/





	bool isRotating = true;

	bool isOrthoCam = true;

	bool isButtonPressed = false;
	bool isButton2Pressed = false;


	//IMGUI

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 460");


	/*static int curOBJ = 0;

	static int totalTransforms = OBJTransform.size();

	std::string objNamestring;

	for (int i = 0; i < OBJTransform.size(); i++) {
		objNamestring += "Object " + i;
		objNamestring += " ";
	}

	float objX, objY, objZ;*/

	//

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);


	///// Game loop /////
	while (!glfwWindowShouldClose(window)) {


		m_DynamicsWorld->stepSimulation(dt, 20);

		glfwSetCursorPosCallback(window, cursor_position_callback);

		// Clear the color and depth buffers
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		m_player->Update(m_cameraDir, m_cameraUp);
		m_player->Draw("u_ModelViewProjection", m_camera->GetViewProjection());

		//o_object_1->Draw("u_ModelViewProjection", m_camera->GetViewProjection());
		o_object_3->Draw("u_ModelViewProjection", m_camera->GetViewProjection());
		//o_object_1->Draw("u_ModelViewProjection", m_camera->GetViewProjection());
		o_object_4->Draw("u_ModelViewProjection", m_camera->GetViewProjection());


		glm::vec3 curCamForward = m_camera.get()->GetForward();

		curCamForward *= 100;

		//glm::vec3 getObjectPos = o_object_2->GetPosition();

		//std::cout << "Object y: " << getObjectPos.y << std::endl;

		

		btCollisionWorld::ClosestRayResultCallback rayCallback(btVector3(m_camera.get()->GetPosition().x, m_camera.get()->GetPosition().y, m_camera.get()->GetPosition().z), btVector3(curCamForward.x, curCamForward.y, curCamForward.z));

		m_DynamicsWorld->rayTest(btVector3(m_camera.get()->GetPosition().x, m_camera.get()->GetPosition().y, m_camera.get()->GetPosition().z), btVector3(curCamForward.x, curCamForward.y, curCamForward.z), rayCallback);

		if (rayCallback.hasHit()) {

			btTransform rayTransform = rayCallback.m_collisionObject->getWorldTransform();

			std::cout << "Ray hit: " << rayTransform.getOrigin().y() << std::endl;

			//if (rayTransform == o_object_2->getTransform()) {
			//	std::cout << "Ray hit object 2" << std::endl;
			//}

		}

		playerMovement();

		glfwPollEvents();

		// WEEK 5: Input handling
		if (glfwGetKey(window, GLFW_KEY_ESCAPE)) {
			if (!isButtonPressed) {
				if (allowMouse) {
					glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
				}
				else {
					glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
				}
				allowMouse = !allowMouse;
				// This is the action we want to perform on key press   

			}    
			isButtonPressed = true;
		} else 
		{
			
			isButtonPressed = false;
		}
		

		// Calculate the time since our last frame (dt)
		double thisFrame = glfwGetTime();
		dt = static_cast<float>(thisFrame - lastFrame);


		// TODO: Week 5 - toggle code

		// Rotate our models around the z axis
		//if (isRotating) {
			//transforms[0]  = glm::rotate(glm::mat4(1.0f), static_cast<float>(thisFrame), glm::vec3(0, 0, 1));
			//transforms[1] = glm::rotate(glm::mat4(1.0f), -static_cast<float>(thisFrame), glm::vec3(0, 0, 1)) * glm::translate(glm::mat4(1.0f), glm::vec3(0, 0.0f, glm::sin(static_cast<float>(thisFrame))));
			//transforms[1] = glm::rotate(glm::mat4(1.0f), static_cast<float>(glm::radians(90.0f)), glm::vec3(1, 0, 0)) + glm::translate(glm::mat4(1.0f), glm::vec3(2,1,0));


			//transforms[1] = glm::rotate(glm::mat4(1.0f), -static_cast<float>(thisFrame), glm::vec3(1, 0, 0)) * glm::translate(glm::mat4(1.0f), glm::vec3(0, glm::sin(static_cast<float>(thisFrame)), 0.0f));
			//transforms[0] = glm::translate(glm::mat4(1.0f), glm::vec3(2, 1, 0));
			//transforms[1] = glm::translate(glm::mat4(1.0f), glm::vec3(-2, -1, 0));
		//}

		//transform4 = glm::rotate(glm::mat4(1.0f), static_cast<float>(thisFrame), glm::vec3(0, 0, 1));
	


		//IMGUI

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		//




		// Bind our shader and upload the uniform
		//shader->Bind();
		
		//Draw models
		//for (int i = 0; i < vaos.size(); i++) {
		//	shader->SetUniformMatrix("u_ModelViewProjection", m_camera->GetViewProjection() * transforms[i]);
		//	vaos[i]->Draw();
		//}

		VertexArrayObject::Unbind();


		//More IMGUI
	

		/*ImGui::Begin("Test1");
		ImGui::Text("Objects:");

		static std::string OBJLabel = "";

		for (size_t i = 0; i < OBJTransform.size(); i++)
		{
			OBJLabel = "Object " + std::to_string(i);

			if (ImGui::Selectable(OBJLabel.c_str(), i == OBJSelected))
			{

				OBJSelected = i;
				objX = OBJTransform[OBJSelected].getOrigin().x();
				objY = OBJTransform[OBJSelected].getOrigin().y();
				objZ = OBJTransform[OBJSelected].getOrigin().z();
			}
		}

		ImGui::SliderFloat("X Pos", &objX, -20.5f, 20.0f);
		ImGui::SliderFloat("Y Pos", &objY, -20.5f, 20.0f);
		ImGui::SliderFloat("Z Pos", &objZ, -20.5f, 20.0f);

		OBJTransform[OBJSelected].setOrigin(btVector3(objX, objY, objZ));


		ImGui::End();*/

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		//

		glfwSwapBuffers(window);
	}

	m_player->CleanUp();

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	delete m_DynamicsWorld;
	delete m_Solver;
	delete m_OverlappingPairCache;
	delete m_CollisionDispatcher;
	delete m_CollisionConfig;

	// Clean up the toolkit logger so we don't leak memory
	Logger::Uninitialize();
	return 0;
}