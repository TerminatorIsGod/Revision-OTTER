#include "Gameplay/Components/AnimationSystemManager.h"
#include <GLFW/glfw3.h>
#include "Gameplay/GameObject.h"
#include "Gameplay/Scene.h"
#include "Utils/ImGuiHelper.h"
#include "Gameplay/Components/pathfindingManager.h"
#include "Gameplay/Physics/RigidBody.h"
#include <Gameplay/Components/RenderComponent.h>

void AnimationSystemManager::Awake()
{

	for (int i = 0; i < frames; i++) {
		std::string tempName = filePrefix + std::to_string(i) + ".obj";
		meshes.push_back(ResourceManager::CreateAsset<MeshResource>(tempName));
	}

}

void AnimationSystemManager::RenderImGui() {

	if (LABEL_LEFT(ImGui::Button, "List Meshes")) {
		listMeshes = true;
	}
}

nlohmann::json AnimationSystemManager::ToJson() const {
	return {
		{ "frames", frames},
		{ "filePrefix", filePrefix}
	};
}

AnimationSystemManager::AnimationSystemManager() :
	IComponent(),
	frames(8),
	filePrefix("animations/test/test")
{ }

AnimationSystemManager::~AnimationSystemManager() = default;

AnimationSystemManager::Sptr AnimationSystemManager::FromJson(const nlohmann::json & blob) {
	AnimationSystemManager::Sptr result = std::make_shared<AnimationSystemManager>();
	result->frames = blob["frames"];
	result->filePrefix = blob["filePrefix"];
	return result;
}

void AnimationSystemManager::Update(float deltaTime) {

	if (listMeshes) {
		listMeshes = false;

		for (int i = 0; i < frames; i++) {
			MeshResource::Sptr tempMesh = meshes.at(i);
			std::cout << "Mesh[" << i << "] = " << tempMesh->Filename << "\n";
		}
	}

}