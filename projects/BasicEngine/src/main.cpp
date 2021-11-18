#include <Logging.h>
#include <iostream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <filesystem>
#include <json.hpp>
#include <fstream>
#include <sstream>
#include <typeindex>
#include <optional>
#include <string>

// GLM math library
#include <GLM/glm.hpp>
#include <GLM/gtc/matrix_transform.hpp>
#include <GLM/gtc/type_ptr.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <GLM/gtx/common.hpp> // for fmod (floating modulus) 

// Graphics
#include "Graphics/IndexBuffer.h"
#include "Graphics/VertexBuffer.h"
#include "Graphics/VertexArrayObject.h"
#include "Graphics/Shader.h"
#include "Graphics/Texture2D.h"
#include "Graphics/TextureCube.h"
#include "Graphics/VertexTypes.h"

// Utilities
#include "Utils/MeshBuilder.h"
#include "Utils/MeshFactory.h"
#include "Utils/ObjLoader.h"
#include "Utils/ImGuiHelper.h"
#include "Utils/ResourceManager/ResourceManager.h"
#include "Utils/FileHelpers.h"
#include "Utils/JsonGlmHelpers.h"
#include "Utils/StringUtils.h"
#include "Utils/GlmDefines.h"

// Gameplay
#include "Gameplay/Material.h"
#include "Gameplay/GameObject.h"
#include "Gameplay/Scene.h"

// Components
#include "Gameplay/Components/IComponent.h"
#include "Gameplay/Components/Camera.h"
#include "Gameplay/Components/RotatingBehaviour.h"
#include "Gameplay/Components/JumpBehaviour.h"
#include "Gameplay/Components/RenderComponent.h"
#include "Gameplay/Components/MaterialSwapBehaviour.h"

#include "Gameplay/Components/InventorySystem.h"
#include "Gameplay/Components/SceneSwapSystem.h"

#include "Gameplay/Components/NavNode.h"
#include "Gameplay/Components/pathfindingManager.h"
#include "Gameplay/Components/SoundEmmiter.h"


// Physics
#include "Gameplay/Physics/RigidBody.h"
#include "Gameplay/Physics/Colliders/BoxCollider.h"
#include "Gameplay/Physics/Colliders/PlaneCollider.h"
#include "Gameplay/Physics/Colliders/SphereCollider.h"
#include "Gameplay/Physics/Colliders/CapsuleCollider.h"
#include "Gameplay/Physics/Colliders/ConvexMeshCollider.h"
#include "Gameplay/Physics/TriggerVolume.h"
#include "Graphics/DebugDraw.h"
#include "Gameplay/Components/TriggerVolumeEnterBehaviour.h"


#include "Gameplay/Components/SimpleCameraControl.h"
#include <Gameplay/Components/MenuSystem.h>



//#define LOG_GL_NOTIFICATIONS

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
std::string windowTitle = "INFR-1350U";

bool isGamePaused = true;
bool isGameStarted = false;

// using namespace should generally be avoided, and if used, make sure it's ONLY in cpp files
using namespace Gameplay;
using namespace Gameplay::Physics;

// The scene that we will be rendering
Scene::Sptr scene = nullptr;

void GlfwWindowResizedCallback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
	windowSize = glm::ivec2(width, height);
	if (windowSize.x * windowSize.y > 0) {
		scene->MainCamera->ResizeWindow(width, height);
	}
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

/// <summary>
/// Draws a widget for saving or loading our scene
/// </summary>
/// <param name="scene">Reference to scene pointer</param>
/// <param name="path">Reference to path string storage</param>
/// <returns>True if a new scene has been loaded</returns>
bool DrawSaveLoadImGui(Scene::Sptr& scene, std::string& path) {
	// Since we can change the internal capacity of an std::string,
	// we can do cool things like this!
	ImGui::InputText("Path", path.data(), path.capacity());

	// Draw a save button, and save when pressed
	if (ImGui::Button("Save")) {
		scene->Save(path);

		std::string newFilename = std::filesystem::path(path).stem().string() + "-manifest.json";
		ResourceManager::SaveManifest(newFilename);
	}
	ImGui::SameLine();
	// Load scene from file button
	if (ImGui::Button("Load")) {
		// Since it's a reference to a ptr, this will
		// overwrite the existing scene!
		scene = nullptr;

		std::string newFilename = std::filesystem::path(path).stem().string() + "-manifest.json";
		ResourceManager::LoadManifest(newFilename);
		scene = Scene::Load(path);

		return true;
	}
	return false;
}

/// <summary>
/// Draws some ImGui controls for the given light
/// </summary>
/// <param name="title">The title for the light's header</param>
/// <param name="light">The light to modify</param>
/// <returns>True if the parameters have changed, false if otherwise</returns>
bool DrawLightImGui(const Scene::Sptr& scene, const char* title, int ix) {
	bool isEdited = false;
	bool result = false;
	Light& light = scene->Lights[ix];
	ImGui::PushID(&light); // We can also use pointers as numbers for unique IDs
	if (ImGui::CollapsingHeader(title)) {
		isEdited |= ImGui::DragFloat3("Pos", &light.Position.x, 0.01f);
		isEdited |= ImGui::ColorEdit3("Col", &light.Color.r);
		isEdited |= ImGui::DragFloat("Range", &light.Range, 0.1f);

		result = ImGui::Button("Delete");
	}
	if (isEdited) {
		scene->SetShaderLight(ix);
	}

	ImGui::PopID();
	return result;
}
int nodeCount = 0;
void createNavNode(glm::vec3 pos, MeshResource::Sptr mesh, Material::Sptr material) {
	nodeCount++;
	GameObject::Sptr navNode = scene->CreateGameObject("Node " + std::to_string(nodeCount));
	{
		// Set position in the scene
		navNode->SetPostion(pos);

		// Create and attach a renderer
		RenderComponent::Sptr renderer = navNode->Add<RenderComponent>();
		renderer->SetMesh(mesh);
		renderer->SetMaterial(material);

		NavNode::Sptr behaviour = navNode->Add<NavNode>();

	}
}



int main() {
	Logger::Init(); // We'll borrow the logger from the toolkit, but we need to initialize it

	//Initialize GLFW
	if (!initGLFW())
		return 1;

	//Initialize GLAD
	if (!initGLAD())
		return 1;

	// Let OpenGL know that we want debug output, and route it to our handler function
	glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	glDebugMessageCallback(GlDebugMessage, nullptr);

	// Initialize our ImGui helper
	ImGuiHelper::Init(window);

	// Initialize our resource manager
	ResourceManager::Init();

	// Register all our resource types so we can load them from manifest files
	ResourceManager::RegisterType<Texture2D>();
	ResourceManager::RegisterType<TextureCube>();
	ResourceManager::RegisterType<Shader>();
	ResourceManager::RegisterType<Material>();
	ResourceManager::RegisterType<MeshResource>();

	// Register all of our component types so we can load them from files
	ComponentManager::RegisterType<Camera>();
	ComponentManager::RegisterType<RenderComponent>();
	ComponentManager::RegisterType<RigidBody>();
	ComponentManager::RegisterType<TriggerVolume>();
	ComponentManager::RegisterType<RotatingBehaviour>();
	ComponentManager::RegisterType<JumpBehaviour>();
	ComponentManager::RegisterType<MaterialSwapBehaviour>();
	ComponentManager::RegisterType<TriggerVolumeEnterBehaviour>();
	ComponentManager::RegisterType<SimpleCameraControl>();
	ComponentManager::RegisterType<NavNode>();
	ComponentManager::RegisterType<pathfindingManager>();
	ComponentManager::RegisterType<SoundEmmiter>();

	ComponentManager::RegisterType<MenuSystem>();


	ComponentManager::RegisterType<InventorySystem>();
	ComponentManager::RegisterType<SceneSwapSystem>();

	// GL states, we'll enable depth testing and backface fulling
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glClearColor(0.2f, 0.2f, 0.2f, 1.0f);

	bool loadScene = false;
	// For now we can use a toggle to generate our scene vs load from file
	if (loadScene) {
		ResourceManager::LoadManifest("manifest.json");
		scene = Scene::Load("demoscene.json");

		// Call scene awake to start up all of our components
		scene->Window = window;
		scene->Awake();
	}
	else {
		// This time we'll have 2 different shaders, and share data between both of them using the UBO
		// This shader will handle reflective materials
		Shader::Sptr reflectiveShader = ResourceManager::CreateAsset<Shader>(std::unordered_map<ShaderPartType, std::string>{
			{ ShaderPartType::Vertex, "shaders/vertex_shader.glsl" },
			{ ShaderPartType::Fragment, "shaders/frag_environment_reflective.glsl" }
		});

		// This shader handles our basic materials without reflections (cause they expensive)
		Shader::Sptr basicShader = ResourceManager::CreateAsset<Shader>(std::unordered_map<ShaderPartType, std::string>{
			{ ShaderPartType::Vertex, "shaders/vertex_shader.glsl" },
			{ ShaderPartType::Fragment, "shaders/frag_blinn_phong_textured.glsl" }
		});

		Shader::Sptr toonShader = ResourceManager::CreateAsset<Shader>(std::unordered_map<ShaderPartType, std::string>{
			{ ShaderPartType::Vertex, "shaders/vertex_shader.glsl" },
			{ ShaderPartType::Fragment, "shaders/toon_shading.glsl" }
		});

		//Meshes
		MeshResource::Sptr monkeyMesh = ResourceManager::CreateAsset<MeshResource>("Monkey.obj");
		MeshResource::Sptr mapMesh = ResourceManager::CreateAsset<MeshResource>("map.obj");
		MeshResource::Sptr mapCollidersMesh = ResourceManager::CreateAsset<MeshResource>("mapColliders.obj");
		MeshResource::Sptr leaflingMesh = ResourceManager::CreateAsset<MeshResource>("Leafling_Ver3_-_Rigged.obj");

		MeshResource::Sptr navNodeMesh = ResourceManager::CreateAsset<MeshResource>("Puck.obj");


		//Textures
		Texture2D::Sptr    boxTexture = ResourceManager::CreateAsset<Texture2D>("textures/box-diffuse.png");
		Texture2D::Sptr    monkeyTex = ResourceManager::CreateAsset<Texture2D>("textures/monkey-uvMap.png");
		Texture2D::Sptr    pinkTex = ResourceManager::CreateAsset<Texture2D>("textures/pink.jpg");
		Texture2D::Sptr    whiteTex = ResourceManager::CreateAsset<Texture2D>("textures/white.jpg");
		Texture2D::Sptr    leaflingTex = ResourceManager::CreateAsset<Texture2D>("textures/Leafling-texture.png");


		// Here we'll load in the cubemap, as well as a special shader to handle drawing the skybox
		TextureCube::Sptr testCubemap = ResourceManager::CreateAsset<TextureCube>("cubemaps/ocean/ocean.jpg");
		Shader::Sptr      skyboxShader = ResourceManager::CreateAsset<Shader>(std::unordered_map<ShaderPartType, std::string>{
			{ ShaderPartType::Vertex, "shaders/skybox_vert.glsl" },
			{ ShaderPartType::Fragment, "shaders/skybox_frag.glsl" }
		});

		// Create an empty scene
		scene = std::make_shared<Scene>();

		// Setting up our enviroment map
		scene->SetSkyboxTexture(testCubemap);
		scene->SetSkyboxShader(skyboxShader);
		// Since the skybox I used was for Y-up, we need to rotate it 90 deg around the X-axis to convert it to z-up
		scene->SetSkyboxRotation(glm::rotate(MAT4_IDENTITY, glm::half_pi<float>(), glm::vec3(1.0f, 0.0f, 0.0f)));

		// Create our materials
		// This will be our box material, with no environment reflections


		Material::Sptr boxMaterial = ResourceManager::CreateAsset<Material>();
		{
			boxMaterial->Name = "Box";
			boxMaterial->MatShader = basicShader;
			boxMaterial->Texture = boxTexture;
			boxMaterial->Shininess = 0.1f;
		}

		// This will be the reflective material, we'll make the whole thing 90% reflective
		Material::Sptr monkeyMaterial = ResourceManager::CreateAsset<Material>();
		{
			monkeyMaterial->Name = "Monkey";
			monkeyMaterial->MatShader = reflectiveShader;
			monkeyMaterial->Texture = monkeyTex;
			monkeyMaterial->Shininess = 1.0f;

		}

		Material::Sptr pinkMaterial = ResourceManager::CreateAsset<Material>();
		{
			pinkMaterial->Name = "Pink";
			pinkMaterial->MatShader = basicShader;
			pinkMaterial->Texture = pinkTex;
			pinkMaterial->Shininess = 1.0f;
		}

		Material::Sptr whiteMaterial = ResourceManager::CreateAsset<Material>();
		{
			whiteMaterial->Name = "White";
			whiteMaterial->MatShader = basicShader;
			whiteMaterial->Texture = whiteTex;
			whiteMaterial->Shininess = 1.0f;
		}

		Material::Sptr leaflingMaterial = ResourceManager::CreateAsset<Material>();
		{
			leaflingMaterial->Name = "Leafling";
			leaflingMaterial->MatShader = basicShader;
			leaflingMaterial->Texture = leaflingTex;
			leaflingMaterial->Shininess = 1.0f;
		}

		Material::Sptr toonMaterial = ResourceManager::CreateAsset<Material>();
		{
			toonMaterial->Name = "Toon";
			toonMaterial->MatShader = toonShader;
			toonMaterial->Texture = whiteTex;
			toonMaterial->Shininess = 1.0f;
		}

		// Create some lights for our scene
		scene->Lights.resize(3);
		scene->Lights[0].Position = glm::vec3(0.0f, 30.0f, 3.0f);
		scene->Lights[0].Color = glm::vec3(1.0f, 1.0f, 1.0f);
		scene->Lights[0].Range = 100.0f;

		scene->Lights[1].Position = glm::vec3(0.0f, 90.0f, 6.0f);
		scene->Lights[1].Color = glm::vec3(1.0f, 1.0f, 1.0f);
		scene->Lights[1].Range = 100.0f;

		scene->Lights[2].Position = glm::vec3(0.0f, 0.0f, 3.0f);
		scene->Lights[2].Color = glm::vec3(1.0f, 0.2f, 0.1f);

		// We'll create a mesh that is a simple plane that we can resize later
		MeshResource::Sptr planeMesh = ResourceManager::CreateAsset<MeshResource>();
		planeMesh->AddParam(MeshBuilderParam::CreatePlane(ZERO, UNIT_Z, UNIT_X, glm::vec2(1.0f)));
		planeMesh->GenerateMesh();

		// Set up the scene's camera
		//At some point the camera should be seperate from player, and parented to it. OR, just make it so the collider doesn't rotate with the transform
		GameObject::Sptr camera = scene->CreateGameObject("Main Camera");
		{
			camera->SetPostion(glm::vec3(5.0f, -5.0f, 7.0f));
			camera->LookAt(glm::vec3(0.0f));

			camera->Add<SimpleCameraControl>();

			Camera::Sptr cam = camera->Add<Camera>();
			// Make sure that the camera is set as the scene's main camera!
			scene->MainCamera = cam;

			//add physics body
			RigidBody::Sptr physics = camera->Add<RigidBody>(RigidBodyType::Dynamic);
			//physics->AddCollider(CapsuleCollider::Create(3.0f, 6.0f));
			physics->AddCollider(SphereCollider::Create(6.0f)); //Switch to capsule collider ASAP


			InventorySystem::Sptr inven = camera->Add<InventorySystem>();

			SoundEmmiter::Sptr emmiter = camera->Add<SoundEmmiter>();
		}

		// Set up all our sample objects
		GameObject::Sptr menu = scene->CreateGameObject("MenuPlane");
		{
			menu->SetPostion(glm::vec3(0.5f, 0.0f, -50.0f));
			menu->SetRotation(glm::vec3(-180.0f, 90.0f, 0.0f));

			RenderComponent::Sptr renderer = menu->Add<RenderComponent>();
			renderer->SetMesh(planeMesh);
			//renderer->SetMaterial(mainmenuMaterial);

			MenuSystem::Sptr menusys = menu->Add<MenuSystem>();
			//menusys->mainScene(scene);
			//menusys->createCamera();
		}

		GameObject::Sptr menuPause = scene->CreateGameObject("MenuPausePlane");
		{
			menuPause->SetPostion(glm::vec3(0.5f, 0.0f, -100.0f));
			menuPause->SetRotation(glm::vec3(-180.0f, 90.0f, 0.0f));

			RenderComponent::Sptr renderer = menuPause->Add<RenderComponent>();
			renderer->SetMesh(planeMesh);
			//renderer->SetMaterial(pausemenuMaterial);

			MenuSystem::Sptr menusys = menuPause->Add<MenuSystem>();
			//menusys->mainScene(scene);
			//menusys->createCamera();
		}

		/*GameObject::Sptr camera = scene->CreateGameObject("Menu Camera");
		{
			camera->SetPostion(glm::vec3(0.0f, 0.0f, -100.0f));
			camera->LookAt(glm::vec3(0.0f));
			camera->SetRotation(glm::vec3(0.0f, -90.0f, 0.0f));

			Camera::Sptr cam = camera->Add<Camera>();
		}*/

		// Set up all our sample objects
		GameObject::Sptr plane = scene->CreateGameObject("Plane");
		{
			//plane->SetScale()
			// Make a big tiled mesh
			MeshResource::Sptr tiledMesh = ResourceManager::CreateAsset<MeshResource>();
			tiledMesh->AddParam(MeshBuilderParam::CreatePlane(ZERO, UNIT_Z, UNIT_X, glm::vec2(100.0f), glm::vec2(20.0f)));
			tiledMesh->GenerateMesh();

			// Create and attach a RenderComponent to the object to draw our mesh
			//RenderComponent::Sptr renderer = plane->Add<RenderComponent>();
			//renderer->SetMesh(tiledMesh);
			//renderer->SetMaterial(boxMaterial);

			// Attach a plane collider that extends infinitely along the X/Y axis
			RigidBody::Sptr physics = plane->Add<RigidBody>(/*static by default*/);
			physics->AddCollider(BoxCollider::Create(glm::vec3(50.0f, 100.0f, 1.0f)))->SetPosition({ 0,0,-1 });
		}

		GameObject::Sptr square = scene->CreateGameObject("Square");
		{
			// Set position in the scene
			square->SetPostion(glm::vec3(0.0f, 0.0f, 2.0f));
			// Scale down the plane
			square->SetScale(glm::vec3(0.5f));

			// Create and attach a render component
			RenderComponent::Sptr renderer = square->Add<RenderComponent>();
			renderer->SetMesh(planeMesh);
			renderer->SetMaterial(boxMaterial);

			// This object is a renderable only, it doesn't have any behaviours or
			// physics bodies attached!
		}


		// Set up all our sample objects
		GameObject::Sptr map = scene->CreateGameObject("Map");
		{
			// Scale up the plane
			map->SetScale(glm::vec3(4.0f, 4.0f, 4.0f));
			map->SetRotation(glm::vec3(90.0f, 0.0f, 0.0f));
			// Create and attach a RenderComponent to the object to draw our mesh
			RenderComponent::Sptr renderer = map->Add<RenderComponent>();
			renderer->SetMesh(mapMesh);
			renderer->SetMaterial(whiteMaterial);

			// Attach a plane collider that extends infinitely along the X/Y axis
			//RigidBody::Sptr physics = plane->Add<RigidBody>(/*static by default*/);
			//physics->AddCollider(PlaneCollider::Create());
		}

		//GameObject::Sptr mapColliders = scene->CreateGameObject("Map");
		//{
		//	// Scale up the plane
		//	mapColliders->SetScale(glm::vec3(4.0f, 4.0f, 4.0f));
		//	mapColliders->SetRotation(glm::vec3(90.0f, 0.0f, 0.0f));

		//	// Create and attach a RenderComponent to the object to draw our mesh
		//	RenderComponent::Sptr renderer = mapColliders->Add<RenderComponent>();
		//	renderer->SetMesh(mapCollidersMesh);
		//	renderer->SetMaterial(whiteMaterial);

		//	//Attach a plane collider that extends infinitely along the X/Y axis
		//	RigidBody::Sptr physics = mapColliders->Add<RigidBody>(/*static by default*/);
		//	physics->AddCollider(ConvexMeshCollider::Create());
		//}

		//Generate Nodes
		for (int x = -2; x < 8; x++)
		{
			for (int y = -2; y < 8; y++)
			{
				createNavNode(glm::vec3(x * 5, y * 5, 1.0f), navNodeMesh, pinkMaterial);
			}
		}


		//Create pathfindingManager, and send it the list of nodes
		GameObject::Sptr PathfindingManager = scene->CreateGameObject("Pathfinding Manager");
		{
			pathfindingManager::Sptr behaviour = PathfindingManager->Add<pathfindingManager>();
		}

		// Create a trigger volume for testing how we can detect collisions with objects!
		GameObject::Sptr trigger = scene->CreateGameObject("Trigger");
		{
			TriggerVolume::Sptr volume = trigger->Add<TriggerVolume>();
			BoxCollider::Sptr collider = BoxCollider::Create(glm::vec3(3.0f, 3.0f, 1.0f));
			collider->SetPosition(glm::vec3(0.0f, 0.0f, 0.5f));
			volume->AddCollider(collider);

			trigger->Add<TriggerVolumeEnterBehaviour>();
		}

		// Call scene awake to start up all of our components
		scene->Window = window;
		scene->Awake();

		// Save the asset manifest for all the resources we just loaded
		ResourceManager::SaveManifest("scene-manifest.json");
		// Save the scene to a JSON file
		scene->Save("scene.json");
	}


	// We'll use this to allow editing the save/load path
	// via ImGui, note the reserve to allocate extra space
	// for input!
	std::string scenePath = "scene.json";
	scenePath.reserve(256);

	bool isRotating = true;
	float rotateSpeed = 90.0f;

	// Our high-precision timer
	double lastFrame = glfwGetTime();


	BulletDebugMode physicsDebugMode = BulletDebugMode::None;
	float playbackSpeed = 1.0f;

	nlohmann::json editorSceneState;

	bool isEscapePressed = false;
	GameObject::Sptr menuPlane;
	MenuSystem::Sptr menuSys;
	Camera::Sptr camera;

	///// Game loop /////
	while (!glfwWindowShouldClose(window)) {



		if (scene->FindObjectByName("MenuPlane") != nullptr) {
			menuPlane = scene->FindObjectByName("MenuPlane");
			menuSys = menuPlane->Get<MenuSystem>();
		}

		//A bunch of checks to make sure it doesn't crash in the case the menus are missing

		if (scene->FindObjectByName("MenuPlane") != nullptr) {

			if (!isGamePaused && isGameStarted)
				camera = scene->MainCamera;
			else {
				if (scene->FindObjectByName("MenuPlane") != nullptr) {
					camera = scene->FindObjectByName("Menu Camera")->Get<Camera>();
					//camera = menuSys->getMenuCamera();

					if (!isGameStarted) {
						camera->GetGameObject()->SetPostion(glm::vec3(0.0f, 0.0f, -50.0f));
					}
					else {
						camera->GetGameObject()->SetPostion(glm::vec3(0.0f, 0.0f, -100.0f));
					}
				}
				else {
					camera = scene->MainCamera;
				}

			}

		}
		else {
			camera = scene->MainCamera;
		}



		//Check to see if pause game
		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
			if (!isEscapePressed && isGameStarted) {
				isGamePaused = !isGamePaused;

			}
			isEscapePressed = true;
		}
		else {
			isEscapePressed = false;
		}

		if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
			if (!isGameStarted) {
				isGameStarted = true;
				isGamePaused = false;
			}
		}


		glfwPollEvents();
		ImGuiHelper::StartFrame();

		// Calculate the time since our last frame (dt)
		double thisFrame = glfwGetTime();
		float dt = static_cast<float>(thisFrame - lastFrame);

		// Showcasing how to use the imGui library!
		bool isDebugWindowOpen = ImGui::Begin("Debugging");
		if (isDebugWindowOpen) {
			// Draws a button to control whether or not the game is currently playing
			static char buttonLabel[64];
			sprintf_s(buttonLabel, "%s###playmode", scene->IsPlaying ? "Exit Play Mode" : "Enter Play Mode");
			ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
			if (ImGui::Button(buttonLabel)) {
				// Save scene so it can be restored when exiting play mode
				if (!scene->IsPlaying) {
					editorSceneState = scene->ToJson();
				}

				// Toggle state
				scene->IsPlaying = !scene->IsPlaying;

				// If we've gone from playing to not playing, restore the state from before we started playing
				if (!scene->IsPlaying) {
					scene = nullptr;
					// We reload to scene from our cached state
					scene = Scene::FromJson(editorSceneState);
					// Don't forget to reset the scene's window and wake all the objects!
					scene->Window = window;
					scene->Awake();
				}
			}

			// Make a new area for the scene saving/loading
			ImGui::Separator();
			if (DrawSaveLoadImGui(scene, scenePath)) {
				// C++ strings keep internal lengths which can get annoying
				// when we edit it's underlying datastore, so recalcualte size
				scenePath.resize(strlen(scenePath.c_str()));

				// We have loaded a new scene, call awake to set
				// up all our components
				scene->Window = window;
				scene->Awake();
			}
			ImGui::Separator();
			// Draw a dropdown to select our physics debug draw mode
			if (BulletDebugDraw::DrawModeGui("Physics Debug Mode:", physicsDebugMode)) {
				scene->SetPhysicsDebugDrawMode(physicsDebugMode);
			}
			LABEL_LEFT(ImGui::SliderFloat, "Playback Speed:    ", &playbackSpeed, 0.0f, 10.0f);
			ImGui::Separator();
		}

		// Clear the color and depth buffers
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Update our application level uniforms every frame

		// Draw some ImGui stuff for the lights
		if (isDebugWindowOpen) {
			ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
			for (int ix = 0; ix < scene->Lights.size(); ix++) {
				char buff[256];
				sprintf_s(buff, "Light %d##%d", ix, ix);

				// DrawLightImGui will return true if the light was deleted
				if (DrawLightImGui(scene, buff, ix)) {
					// Remove light from scene, restore all lighting data
					scene->Lights.erase(scene->Lights.begin() + ix);
					scene->SetupShaderAndLights();
					// Move back one element so we don't skip anything!
					ix--;
				}
			}
			// As long as we don't have max lights, draw a button
			// to add another one
			if (scene->Lights.size() < scene->MAX_LIGHTS) {
				if (ImGui::Button("Add Light")) {
					scene->Lights.push_back(Light());
					scene->SetupShaderAndLights();
				}
			}
			// Split lights from the objects in ImGui
			ImGui::Separator();
		}

		dt *= playbackSpeed;

		// Perform updates for all components
		if (!isGamePaused) //doesn't update components if its paused
			scene->Update(dt);

		// Cache the camera's viewprojection
		glm::mat4 viewProj = camera->GetViewProjection();
		DebugDrawer::Get().SetViewProjection(viewProj);

		// Update our worlds physics!
		if (!isGamePaused) //doesn't update physics if its paused
			scene->DoPhysics(dt);

		// Draw object GUIs
		if (isDebugWindowOpen) {
			scene->DrawAllGameObjectGUIs();
		}

		// The current material that is bound for rendering
		Material::Sptr currentMat = nullptr;
		Shader::Sptr shader = nullptr;

		TextureCube::Sptr environment = scene->GetSkyboxTexture();
		if (environment) environment->Bind(0);

		// Render all our objects
		ComponentManager::Each<RenderComponent>([&](const RenderComponent::Sptr& renderable) {
			// Early bail if mesh not set
			if (renderable->GetMesh() == nullptr) {
				return;
			}

			// If we don't have a material, try getting the scene's fallback material
			// If none exists, do not draw anything
			if (renderable->GetMaterial() == nullptr) {
				if (scene->DefaultMaterial != nullptr) {
					renderable->SetMaterial(scene->DefaultMaterial);
				}
				else {
					return;
				}
			}

			// If the material has changed, we need to bind the new shader and set up our material and frame data
			// Note: This is a good reason why we should be sorting the render components in ComponentManager
			if (renderable->GetMaterial() != currentMat) {
				currentMat = renderable->GetMaterial();
				shader = currentMat->MatShader;

				shader->Bind();
				shader->SetUniform("u_CamPos", scene->MainCamera->GetGameObject()->GetPosition());
				currentMat->Apply();
			}

			// Grab the game object so we can do some stuff with it
			GameObject* object = renderable->GetGameObject();

			// Set vertex shader parameters
			shader->SetUniformMatrix("u_ModelViewProjection", viewProj * object->GetTransform());
			shader->SetUniformMatrix("u_Model", object->GetTransform());
			shader->SetUniformMatrix("u_NormalMatrix", glm::mat3(glm::transpose(glm::inverse(object->GetTransform()))));

			// Draw the object
			renderable->GetMesh()->Draw();
			});

		// Use our cubemap to draw our skybox
		scene->DrawSkybox();


		// End our ImGui window
		ImGui::End();

		VertexArrayObject::Unbind();

		lastFrame = thisFrame;
		ImGuiHelper::EndFrame();
		glfwSwapBuffers(window);
	}

	// Clean up the ImGui library
	ImGuiHelper::Cleanup();

	// Clean up the resource manager
	ResourceManager::Cleanup();

	// Clean up the toolkit logger so we don't leak memory
	Logger::Uninitialize();
	return 0;
}