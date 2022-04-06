#include "Application/Application.h"

#include <Windows.h>
#include <GLFW/glfw3.h> 
#include <glad/glad.h> 

#include "Logging.h"     
#include "Gameplay/InputEngine.h"
#include "Application/Timing.h" 
#include <filesystem>
#include "Layers/GLAppLayer.h"
#include "Utils/FileHelpers.h"
#include "Utils/ResourceManager/ResourceManager.h"
#include "Utils/ImGuiHelper.h"

// Graphics  
#include "Graphics/Buffers/IndexBuffer.h"
#include "Graphics/Buffers/VertexBuffer.h"
#include "Graphics/VertexArrayObject.h"
#include "Graphics/ShaderProgram.h"
#include "Graphics/Textures/Texture1D.h"
#include "Graphics/Textures/Texture2D.h"
#include "Graphics/Textures/Texture3D.h"
#include "Graphics/Textures/TextureCube.h"
#include "Graphics/VertexTypes.h"
#include "Graphics/Font.h"
#include "Graphics/GuiBatcher.h"
#include "Graphics/Framebuffer.h"

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
#include "Gameplay/Components/TriggerVolumeEnterBehaviour.h"
#include "Gameplay/Components/SimpleCameraControl.h"
#include "Gameplay/Components/ParticleSystem.h"
#include "Gameplay/Components/Light.h"
#include "Gameplay/Components/ShadowCamera.h"

#include "Gameplay/Components/InventorySystem.h"
#include "Gameplay/Components/NavNode.h"
#include "Gameplay/Components/pathfindingManager.h"
#include "Gameplay/Components/SoundEmmiter.h"
#include "Gameplay/Components/Enemy.h"
#include "Gameplay/Components/Ladder.h"
#include "Gameplay/Components/UIElement.h"
#include <Gameplay/Components/MenuSystem.h>
#include <Gameplay/Components/InteractSystem.h> 
#include <Gameplay/Components/LerpSystem.h> 
#include <Gameplay/Components/CurveLerpSystem.h>
#include <Gameplay/Components/MenuSystemNewAndImproved.h>
#include <Gameplay/Components/AudioManager.h>
#include <Gameplay/Components/NoteSystem.h>
#include <Gameplay/Components/ThrowableItem.h>
#include <Gameplay/Components/AnimationSystem.h>
#include <Gameplay/Components/SlideLerpSystem.h> 
#include <Gameplay/Components/AnimationSystemManager.h>

// GUI
#include "Gameplay/Components/GUI/RectTransform.h"
#include "Gameplay/Components/GUI/GuiPanel.h"
#include "Gameplay/Components/GUI/GuiText.h"
#include "Gameplay/Components/ComponentManager.h"

// Layers
#include "Layers/RenderLayer.h"
#include "Layers/InterfaceLayer.h"
#include "Layers/DefaultSceneLayer.h" 
#include "Layers/LogicUpdateLayer.h"
#include "Layers/ImGuiDebugLayer.h"
#include "Layers/InstancedRenderingTestLayer.h"
#include "Layers/ParticleLayer.h"
#include "Layers/PostProcessingLayer.h"  
#include <future>
#include <fstream>
#include "Utils/ObjLoader.h"

Application* Application::_singleton = nullptr;
std::string Application::_applicationName = "Resonance";
static std::string asyncFileName = "";
int currentAsyncItem = 0;
static std::vector<std::string> asyncObjectFileNames;

#define DEFAULT_WINDOW_WIDTH 1920 
#define DEFAULT_WINDOW_HEIGHT 1080

bool Application::PassiveLoadFiles() {

	//ObjLoader loader;
	std::cout << "starting async thread: " << std::this_thread::get_id() << std::endl;

	bool wasAbleToLoad = true;

	if (asyncObjectFileNames.empty()) {
		wasAbleToLoad = false;
		LOG_ERROR("Unable to load async, vector was empty!");
		return false;
	}
		

	while (!asyncObjectFileNames.empty()) {

		std::string str;
		str = asyncObjectFileNames.front();
		asyncObjectFileNames.erase(asyncObjectFileNames.begin());

		std::cout << "Loading asset async, Object: " << str << "    Thread number: " << std::this_thread::get_id() << std::endl;
		ObjLoader::LoadFromFile(str, true, true);
	}

	/*if (std::filesystem::exists(file)) {

		std::ifstream in(file);
		std::string str;

		if (in) {
			while (std::getline(in, str)) {
				std::cout << "Loading asset async: " << file << " Object: " << str << "    Thread number: " << std::this_thread::get_id() << std::endl;
				ObjLoader::LoadFromFile(str, true, true);
				//ResourceManager::CreateAsset<MeshResource>(str);
			}

			return true;
		}

	}
		
	LOG_ERROR("Unable to load async, file doesn't exist! File: " + file);
	
	*/

	return true;

}

Application::Application() :
	_window(nullptr),
	_windowSize({ DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT }),
	_isRunning(false),
	_isEditor(true),
	_windowTitle("Resonance"),
	_currentScene(nullptr),
	_targetScene(nullptr)
{ }

Application::~Application() = default;

Application& Application::Get() {
	LOG_ASSERT(_singleton != nullptr, "Failed to get application! Get was called before the application was started!");
	return *_singleton;
}

void Application::Start(int argCount, char** arguments) {
	LOG_ASSERT(_singleton == nullptr, "Application has already been started!");
	_singleton = new Application();
	_singleton->_Run();
}

GLFWwindow* Application::GetWindow() { return _window; }

const glm::ivec2& Application::GetWindowSize() const { return _windowSize; }


const glm::uvec4& Application::GetPrimaryViewport() const {
	return _primaryViewport;
}

void Application::SetPrimaryViewport(const glm::uvec4 & value) {
	_primaryViewport = value;
}

void Application::ResizeWindow(const glm::ivec2 & newSize)
{
	_HandleWindowSizeChanged(newSize);
}

void Application::Quit() {
	_isRunning = false;
}

bool Application::LoadScene(const std::string & path) {
	if (std::filesystem::exists(path)) {
		isEscapePressed = false;
		isGamePaused = false;
		isGameStarted = true;
		scenePath = path;
		std::string manifestPath = std::filesystem::path(path).stem().string() + "-manifest.json";
		if (std::filesystem::exists(manifestPath)) {
			LOG_INFO("Loading manifest from \"{}\"", manifestPath);
			ResourceManager::LoadManifest(manifestPath);
		}

		Gameplay::Scene::Sptr scene = Gameplay::Scene::Load(path);
		LoadScene(scene);
		return scene != nullptr;
		scene->IsPlaying = true;
		_backupState = scene->ToJson();
	}
	return false;
}

void Application::LoadScene(const Gameplay::Scene::Sptr & scene) {
	_targetScene = scene;
}

void Application::SaveSettings()
{
	std::filesystem::path appdata = getenv("APPDATA");
	std::filesystem::path settingsPath = appdata / _applicationName / "app-settings.json";

	if (!std::filesystem::exists(appdata / _applicationName)) {
		std::filesystem::create_directory(appdata / _applicationName);
	}

	FileHelpers::WriteContentsToFile(settingsPath.string(), _appSettings.dump(1, '\t'));
}

void Application::_Run()
{

	asyncFileName = "async/allAssetsList.txt";


	if (std::filesystem::exists(asyncFileName)) {

		std::ifstream in(asyncFileName);
		std::string str;

		if (in) {
			while (std::getline(in, str)) {
				asyncObjectFileNames.push_back(str);
			}
		}

	}
	else {
		LOG_ERROR("Unable to load async, file doesn't exist! File: " + asyncFileName);
	}


	std::future<bool> loadAsync1 = std::async(std::launch::async, PassiveLoadFiles);
	std::future<bool> loadAsync2 = std::async(std::launch::async, PassiveLoadFiles);
	std::future<bool> loadAsync3 = std::async(std::launch::async, PassiveLoadFiles);
	std::future<bool> loadAsync4 = std::async(std::launch::async, PassiveLoadFiles);
	std::future<bool> loadAsync5 = std::async(std::launch::async, PassiveLoadFiles);
	
	// TODO: Register layers
	_layers.push_back(std::make_shared<GLAppLayer>());
	_layers.push_back(std::make_shared<LogicUpdateLayer>());
	_layers.push_back(std::make_shared<RenderLayer>());
	_layers.push_back(std::make_shared<ParticleLayer>());
	_layers.push_back(std::make_shared<PostProcessingLayer>());
	_layers.push_back(std::make_shared<InterfaceLayer>());

	// If we're in editor mode, we add all the editor layers
	if (_isEditor) {
		_layers.push_back(std::make_shared<ImGuiDebugLayer>());
	}

	_layers.push_back(std::make_shared<DefaultSceneLayer>());

	// Either load the settings, or use the defaults
	_ConfigureSettings();

	// We'll grab these since we'll need them!
	_windowSize.x = JsonGet(_appSettings, "window_width", DEFAULT_WINDOW_WIDTH);
	_windowSize.y = JsonGet(_appSettings, "window_height", DEFAULT_WINDOW_HEIGHT);

	// By default, we want our viewport to be the whole screen
	_primaryViewport = { 0, 0, _windowSize.x, _windowSize.y };

	// Register all component and resource types
	_RegisterClasses();


	// Load all layers
	_Load();



	//do multithreaded loading
	//std::future<bool> loadAsync1 = std::async(std::launch::async, PassiveLoadFiles, "async/task1.txt");

	//std::future<bool> loadAsync2 = std::async(std::launch::async, PassiveLoadFiles, "async/task2.txt");


	// Grab current time as the previous frame
	double lastFrame = glfwGetTime();

	// Done loading, app is now running!
	_isRunning = true;

	bool isSwappingScenesCur = false; //used in order to update UI for loading screen

	bool isBeingInteracted = false;

	bool changeSensToGlobal = true;


	// Infinite loop as long as the application is running
	while (_isRunning) {
		// Handle scene switching
		if (_targetScene != nullptr) {
			changeSensToGlobal = true;
			_HandleSceneChange();
		}

		if ((_currentScene->FindObjectByName("StartScreenPlane") && glfwGetKey(_window, GLFW_KEY_SPACE) == GLFW_PRESS) || isSwappingScenesCur) {
			_currentScene->FindObjectByName("StartScreenPlane")->Get<RenderComponent>()->IsEnabled = false;
			_currentScene->FindObjectByName("LoadingScreenPlane")->Get<RenderComponent>()->IsEnabled = true;
			if (isSwappingScenesCur) { //makes sure loading screen is showing before actually loading
				isSwappingScenesCur = false;
				_currentScene->audioManager->Get<AudioManager>()->UnloadSound("Title");
				_currentScene->audioManager->Get<AudioManager>()->PlaySoundByName("Transition");
				_currentScene->audioManager->Get<AudioManager>()->studioSystem->update();

				LoadScene("level1.json");
			}
			else {
				isSwappingScenesCur = true;
			}
		}

		/*if (_currentScene->FindObjectByName("Main Camera")) {
			const auto& cam = _currentScene->MainCamera;
			glm::vec3 v1 = cam->GetGameObject()->GetPosition();
			glm::vec3 v2 = cam->GetComponent<SimpleCameraControl>()->WhatAreYouLookingAt();

			float dist = sqrt((pow((v2.x - v1.x), 2) + pow((v2.y - v1.y), 2)) + pow((v2.z - v1.z), 2));

			if (v2 == glm::vec3(0.0f))
				dist = 100.0f;

			cam->FocalDepth = dist;
		}*/

		//if (glfwGetKey(_window, GLFW_KEY_0) == GLFW_PRESS) {
		//	LoadScene("levelMenu.json");
		//} 
		//else if (glfwGetKey(_window, GLFW_KEY_1) == GLFW_PRESS) {
		//	LoadScene("level1.json");
		//} 

		//Check to see if pause game
		if (glfwGetKey(_window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
			if (!isEscapePressed && isGameStarted) {
				isGamePaused = !isGamePaused;
				showPauseScreen = isGamePaused;
			}
			isEscapePressed = true;
		}
		else {
			isEscapePressed = false;
		}

		if (_currentScene->FindObjectByName("PauseScreen") != nullptr) {
			auto menuobj = _currentScene->FindObjectByName("PauseScreen");
			//auto menusystem = menuobj->Get<MenuSystemNewAndImproved>();
			auto menuenablething = menuobj->Get<GuiPanel>();


			if (isGamePaused && showPauseScreen) {
				menuenablething->IsEnabled = true;
			}
			else {
				menuenablething->IsEnabled = false;
			}
		}


		//do sensitivity stuff
		if (changeSensToGlobal) {
			if (_currentScene->FindObjectByName("Main Camera")) {
				if (globalSens != glm::vec2(0.0f, 0.0f)) {
					_currentScene->FindObjectByName("Main Camera")->Get<SimpleCameraControl>()->_mouseSensitivity = globalSens;
				}
			}

			changeSensToGlobal = false;
		}


		if (isGamePaused && showPauseScreen && _currentScene->FindObjectByName("Main Camera")) {

			if ((glfwGetKey(_window, GLFW_KEY_LEFT) == GLFW_PRESS) || (glfwGetKey(_window, GLFW_KEY_RIGHT) == GLFW_PRESS)) {

				if (!isBeingInteracted) {
					isBeingInteracted = true;

					glm::vec2 sens = _currentScene->FindObjectByName("Main Camera")->Get<SimpleCameraControl>()->_mouseSensitivity;

					if ((glfwGetKey(_window, GLFW_KEY_LEFT) == GLFW_PRESS) && (sens.x >= 0.026))
						sens += glm::vec2(-0.025, -0.025);

					if ((glfwGetKey(_window, GLFW_KEY_RIGHT) == GLFW_PRESS))
						sens += glm::vec2(0.025, 0.025);

					_currentScene->FindObjectByName("Main Camera")->Get<SimpleCameraControl>()->_mouseSensitivity = sens;
					globalSens = sens;

					std::cout << "New Sensitivity: " << sens.x << " " << sens.y << std::endl;
				}

			}
			else {
				isBeingInteracted = false;
			}

		}


		if (isInteracting && glfwGetKey(_window, GLFW_KEY_E) != GLFW_PRESS)
			isInteracting = false;

		if (isGamePaused && !isInteracting && !showPauseScreen && glfwGetKey(_window, GLFW_KEY_E) == GLFW_PRESS) {
			isGamePaused = false;
		}

		if (_currentScene->requestSceneReload && glfwGetKey(GetWindow(), GLFW_KEY_E)) {
			LoadScene(scenePath);
			_currentScene->IsPlaying = true;
		}

		// Receive events like input and window position/size changes from GLFW
		glfwPollEvents();

		// Handle closing the app via the close button
		if (glfwWindowShouldClose(_window)) {
			_isRunning = false;
		}

		// Grab the timing singleton instance as a reference
		Timing& timing = Timing::_singleton;

		// Figure out the current time, and the time since the last frame
		double thisFrame = glfwGetTime();
		float dt = static_cast<float>(thisFrame - lastFrame);
		float scaledDt = dt * timing._timeScale;

		// Update all timing values
		timing._unscaledDeltaTime = dt;
		timing._deltaTime = scaledDt;
		timing._timeSinceAppLoad += scaledDt;
		timing._unscaledTimeSinceAppLoad += dt;
		timing._timeSinceSceneLoad += scaledDt;
		timing._unscaledTimeSinceSceneLoad += dt;

		ImGuiHelper::StartFrame();

		// Core update loop
		if (_currentScene != nullptr) {
			_Update();
			_LateUpdate();
			_PreRender();
			_RenderScene();
			_PostRender();
		}

		// Store timing for next loop
		lastFrame = thisFrame;

		InputEngine::EndFrame();
		ImGuiHelper::EndFrame();

		glfwSwapBuffers(_window);

	}

	// Unload all our layers
	_Unload();
}

void Application::_RegisterClasses()
{
	using namespace Gameplay;
	using namespace Gameplay::Physics;

	// Initialize our resource manager
	ResourceManager::Init();

	// Register all our resource types so we can load them from manifest files
	ResourceManager::RegisterType<Texture1D>();
	ResourceManager::RegisterType<Texture2D>();
	ResourceManager::RegisterType<Texture3D>();
	ResourceManager::RegisterType<TextureCube>();
	ResourceManager::RegisterType<ShaderProgram>();
	ResourceManager::RegisterType<Material>();
	ResourceManager::RegisterType<MeshResource>();
	ResourceManager::RegisterType<Font>();
	ResourceManager::RegisterType<Framebuffer>();

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
	ComponentManager::RegisterType<RectTransform>();
	ComponentManager::RegisterType<GuiPanel>();
	ComponentManager::RegisterType<GuiText>();
	ComponentManager::RegisterType<ParticleSystem>();
	ComponentManager::RegisterType<Light>();
	ComponentManager::RegisterType<ShadowCamera>();

	ComponentManager::RegisterType<NavNode>();
	ComponentManager::RegisterType<pathfindingManager>();
	ComponentManager::RegisterType<SoundEmmiter>();
	ComponentManager::RegisterType<Enemy>();
	ComponentManager::RegisterType<Ladder>();
	ComponentManager::RegisterType<UIElement>();

	ComponentManager::RegisterType<MenuSystem>();
	ComponentManager::RegisterType<InventorySystem>();
	ComponentManager::RegisterType<InteractSystem>();
	ComponentManager::RegisterType<LerpSystem>();
	ComponentManager::RegisterType<CurveLerpSystem>();
	ComponentManager::RegisterType<MenuSystemNewAndImproved>();
	ComponentManager::RegisterType<AudioManager>();
	ComponentManager::RegisterType<NoteSystem>();
	ComponentManager::RegisterType<ThrowableItem>();


	ComponentManager::RegisterType<AnimationSystem>();
	ComponentManager::RegisterType<AnimationSystemManager>();
	ComponentManager::RegisterType<SlideLerpSystem>();
}

void Application::_Load() {
	for (const auto& layer : _layers) {
		if (layer->Enabled && *(layer->Overrides & AppLayerFunctions::OnAppLoad)) {
			layer->OnAppLoad(_appSettings);
		}
	}

	//glfwSwapInterval(1);//Locks framerate to monitors refresh rate

	// Pass the window to the input engine and let it initialize itself
	InputEngine::Init(_window);

	// Initialize our ImGui helper
	ImGuiHelper::Init(_window);

	GuiBatcher::SetWindowSize(_windowSize);
}

void Application::_Update() {
	for (const auto& layer : _layers) {
		if (layer->Enabled && *(layer->Overrides & AppLayerFunctions::OnUpdate)) {
			layer->OnUpdate();
		}
	}
}

void Application::_LateUpdate() {
	for (const auto& layer : _layers) {
		if (layer->Enabled && *(layer->Overrides & AppLayerFunctions::OnLateUpdate)) {
			layer->OnLateUpdate();
		}
	}
}

void Application::_PreRender()
{
	glm::ivec2 size = { 0, 0 };
	glfwGetWindowSize(_window, &size.x, &size.y);
	glViewport(0, 0, size.x, size.y);
	glScissor(0, 0, size.x, size.y);

	// Clear the screen
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	for (const auto& layer : _layers) {
		if (layer->Enabled && *(layer->Overrides & AppLayerFunctions::OnPreRender)) {
			layer->OnPreRender();
		}
	}
}

void Application::_RenderScene() {

	Framebuffer::Sptr result = nullptr;
	for (const auto& layer : _layers) {
		if (layer->Enabled && *(layer->Overrides & AppLayerFunctions::OnRender)) {
			layer->OnRender(result);
		}
	}
}

void Application::_PostRender() {
	// Note that we use a reverse iterator for post render
	for (auto it = _layers.begin(); it != _layers.end(); it++) {
		const auto& layer = *it;
		if (layer->Enabled && *(layer->Overrides & AppLayerFunctions::OnPostRender)) {
			layer->OnPostRender();
		}
	}
}

void Application::_Unload() {
	// Note that we use a reverse iterator for unloading
	for (auto it = _layers.crbegin(); it != _layers.crend(); it++) {
		const auto& layer = *it;
		if (layer->Enabled && *(layer->Overrides & AppLayerFunctions::OnAppUnload)) {
			layer->OnAppUnload();
		}
	}

	// Clean up ImGui
	ImGuiHelper::Cleanup();
}

void Application::_HandleSceneChange() {
	// If we currently have a current scene, let the layers know it's being unloaded
	if (_currentScene != nullptr) {
		// Note that we use a reverse iterator, so that layers are unloaded in the opposite order that they were loaded
		for (auto it = _layers.crbegin(); it != _layers.crend(); it++) {
			const auto& layer = *it;
			if (layer->Enabled && *(layer->Overrides & AppLayerFunctions::OnSceneUnload)) {
				layer->OnSceneUnload();
			}
		}
	}

	_currentScene = _targetScene;

	// Let the layers know that we've loaded in a new scene
	for (const auto& layer : _layers) {
		if (layer->Enabled && *(layer->Overrides & AppLayerFunctions::OnSceneLoad)) {
			layer->OnSceneLoad();
		}
	}

	// Wake up all game objects in the scene
	_currentScene->Awake();

	// If we are not in editor mode, scenes play by default
	if (!_isEditor) {
		_currentScene->IsPlaying = true;
	}

	_targetScene = nullptr;
}

void Application::_HandleWindowSizeChanged(const glm::ivec2 & newSize) {
	for (const auto& layer : _layers) {
		if (layer->Enabled && *(layer->Overrides & AppLayerFunctions::OnWindowResize)) {
			layer->OnWindowResize(_windowSize, newSize);
		}
	}
	_windowSize = newSize;
	_primaryViewport = { 0, 0, newSize.x, newSize.y };
}

void Application::_ConfigureSettings() {
	// Start with the defaul application settings
	_appSettings = _GetDefaultAppSettings();

	// We'll store our settings in the %APPDATA% directory, under our application name
	std::filesystem::path appdata = getenv("APPDATA");
	std::filesystem::path settingsPath = appdata / _applicationName / "app-settings.json";

	// If the settings file exists, we can load it in!
	if (std::filesystem::exists(settingsPath)) {
		// Read contents and parse into a JSON blob
		std::string content = FileHelpers::ReadFile(settingsPath.string());
		nlohmann::json blob = nlohmann::json::parse(content);

		// We use merge_patch so that we can keep our defaults if they are missing from the file!
		_appSettings.merge_patch(blob);
	}
	// If the file does not exist, save the default application settings to the path
	else {
		SaveSettings();
	}
}

nlohmann::json Application::_GetDefaultAppSettings()
{
	nlohmann::json result = {};

	for (const auto& layer : _layers) {
		if (!layer->Name.empty()) {
			result[layer->Name] = layer->GetDefaultConfig();
		}
		else {
			LOG_WARN("Unnamed layer! Injecting settings into global namespace, may conflict with other layers!");
			result.merge_patch(layer->GetDefaultConfig());
		}
	}

	result["window_width"] = DEFAULT_WINDOW_WIDTH;
	result["window_height"] = DEFAULT_WINDOW_HEIGHT;
	return result;
}

