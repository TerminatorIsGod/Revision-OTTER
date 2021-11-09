#include "Gameplay/Components/SceneSwapSystem.h"
#include <GLFW/glfw3.h>
#include "Gameplay/GameObject.h"
#include "Gameplay/Scene.h"
#include "Utils/ImGuiHelper.h"

void SceneSwapSystem::Awake()
{
	
}

void SceneSwapSystem::RenderImGui() {
	//LABEL_LEFT(ImGui::DragInt, "Keys", &_keys, 1.0f);
}

nlohmann::json SceneSwapSystem::ToJson() const {
	return {
		//{ "scene", _scene }
	};
}

SceneSwapSystem::SceneSwapSystem() :
	IComponent(),
	_scene(0)
{ }

SceneSwapSystem::~SceneSwapSystem() = default;

SceneSwapSystem::Sptr SceneSwapSystem::FromJson(const nlohmann::json& blob) {
	//SceneSwapSystem::Sptr result = std::make_shared<SceneSwapSystem>();
	//result->_scene = blob["scene"];
	return 0;//result;
}

void SceneSwapSystem::Update(float deltaTime) {
	
}

void SceneSwapSystem::swapScene(std::string& path) {
	_scene = nullptr;

	std::string newFilename = std::filesystem::path(path).stem().string() + "-manifest.json";
	ResourceManager::LoadManifest(newFilename);
	_scene = Gameplay::Scene::Load(path);
}

void SceneSwapSystem::setScene(Gameplay::Scene::Sptr scene) {
	_scene = scene;
}

Gameplay::Scene::Sptr SceneSwapSystem::getScene() {
	return _scene;
}

