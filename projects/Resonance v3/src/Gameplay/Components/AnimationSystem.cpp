#include "Gameplay/Components/AnimationSystem.h"
#include <GLFW/glfw3.h>
#include "Gameplay/GameObject.h"
#include "Gameplay/Scene.h"
#include "Utils/ImGuiHelper.h"
#include "Gameplay/Components/pathfindingManager.h"
#include "Gameplay/Physics/RigidBody.h"
#include <Gameplay/Components/RenderComponent.h>
#include <Gameplay/Components/AnimationSystemManager.h>

void AnimationSystem::Awake()
{ 

	//for (int i = 0; i < frames; i++) {
	//	std::string tempName = filePrefix + std::to_string(i) + ".obj";
	//	meshes.push_back(ResourceManager::CreateAsset<MeshResource>(tempName));
	//}

} 

void AnimationSystem::RenderImGui() {

	LABEL_LEFT(ImGui::DragFloat, "Speed/FPS", &speed);
	if (LABEL_LEFT(ImGui::Button, "Show Info")) {
		showInfo = true;
	}
	if (LABEL_LEFT(ImGui::Button, "List Meshes")) {
		listMeshes = true;
	}
} 

nlohmann::json AnimationSystem::ToJson() const {
	return {
		{ "speed", speed },
		//{ "frames", frames},
		//{ "filePrefix", filePrefix},
		{ "objectLoadName", objectLoadName}
	};
}

AnimationSystem::AnimationSystem() :
	IComponent(),
	speed(1.0f),
	frames(0),
	filePrefix("animations/test/test"),
	objectLoadName("")
{ }

AnimationSystem::~AnimationSystem() = default;

AnimationSystem::Sptr AnimationSystem::FromJson(const nlohmann::json & blob) {
	AnimationSystem::Sptr result = std::make_shared<AnimationSystem>();
	result->speed = blob["speed"];
	//result->frames = blob["frames"];
	//result->filePrefix = blob["filePrefix"];
	result->objectLoadName = blob["objectLoadName"];
	return result;
}

void AnimationSystem::Update(float deltaTime) {
	//std::cout << "speed = " << speed << " last speed = " << lastSpeed << "\n\n";

	if (!isMeshesLoaded) {
		if (GetGameObject()->GetScene()->FindObjectByName(objectLoadName)) {
			if (GetGameObject()->GetScene()->FindObjectByName(objectLoadName)->Get<AnimationSystemManager>()) {
				meshes = GetGameObject()->GetScene()->FindObjectByName(objectLoadName)->Get<AnimationSystemManager>()->meshes;
				frames = meshes.size();
				isMeshesLoaded = true;
				
			}
		}
				
	}
	else {

		timer += deltaTime;

		if (timer >= (1 / speed)) {
			timer -= (1 / speed);

			GetGameObject()->Get<RenderComponent>()->SetMesh(meshes.at(curFrame));

			curFrame++;
			if (curFrame >= frames)
				curFrame = 0;
		}

		if (showInfo) {
			showInfo = false;
			std::cout << "\n Meshes Count: " << meshes.size() << "\n Current Frame: " << curFrame << "\n Total Frames: " << frames << "\n Speed/FPS: " << speed << "\n\n";
		}

		if (listMeshes) {
			listMeshes = false;

			for (int i = 0; i < frames; i++) {
				MeshResource::Sptr tempMesh = meshes.at(i);
				std::cout << "Mesh[" << i << "] = " << tempMesh->Filename << "\n";
			}
		}

	}

}