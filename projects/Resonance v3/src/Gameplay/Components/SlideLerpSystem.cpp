#include "Gameplay/Components/SlideLerpSystem.h"
#include <GLFW/glfw3.h>
#include "Gameplay/GameObject.h"
#include "Gameplay/Scene.h"
#include "Utils/ImGuiHelper.h"
#include "Gameplay/Components/pathfindingManager.h"
#include "Gameplay/Physics/RigidBody.h"

void SlideLerpSystem::Awake()
{

}

void SlideLerpSystem::RenderImGui() {
	ImGuiStorage* guiStore = ImGui::GetStateStorage();

	if (LABEL_LEFT(ImGui::DragFloat3, "Start Position", &startx, 1.0f)) {

		// Update the editor state with our new values
		guiStore->SetFloat(ImGui::GetID(&startx), startx);
		guiStore->SetFloat(ImGui::GetID(&starty), starty);
		guiStore->SetFloat(ImGui::GetID(&startz), startz);
	}

	if (LABEL_LEFT(ImGui::DragFloat3, "End Position", &endx, 1.0f)) {

		// Update the editor state with our new values
		guiStore->SetFloat(ImGui::GetID(&endx), endx);
		guiStore->SetFloat(ImGui::GetID(&endy), endy);
		guiStore->SetFloat(ImGui::GetID(&endz), endz);
	}

	LABEL_LEFT(ImGui::DragFloat, "How long", &tLength);

	LABEL_LEFT(ImGui::Checkbox, "Update AI", &doUpdateNbors);

}

nlohmann::json SlideLerpSystem::ToJson() const {
	return {
		{ "startx", startx },
		{ "starty", starty },
		{ "startz", startz },
		{ "endx", endx},
		{ "endy", endy},
		{ "endz",  endz},
		{ "tlength", tLength},
		{ "updateAI", doUpdateNbors},
		{ "linkedDoor", linkedDoor}
	};
}

SlideLerpSystem::SlideLerpSystem() :
	IComponent(),
	startx(0),
	starty(0),
	startz(0),
	endx(0),
	endy(0),
	endz(0),
	tLength(1),
	doUpdateNbors(0),
	linkedDoor("NULL")
{ }

SlideLerpSystem::~SlideLerpSystem() = default;

SlideLerpSystem::Sptr SlideLerpSystem::FromJson(const nlohmann::json & blob) {
	SlideLerpSystem::Sptr result = std::make_shared<SlideLerpSystem>();
	result->startx = blob["startx"];
	result->starty = blob["starty"];
	result->startz = blob["startz"];
	result->endx = blob["endx"];
	result->endy = blob["endy"];
	result->endz = blob["endz"];
	result->tLength = blob["tlength"];
	result->doUpdateNbors = blob["updateAI"];
	result->linkedDoor = blob["linkedDoor"];
	return result;
}

void SlideLerpSystem::Update(float deltaTime) {

	if (beginLerp) {
		auto _body = GetComponent<Gameplay::Physics::RigidBody>();
		//if(_body != nullptr)
		//	_body->SetType(RigidBodyType::Kinematic);

		if (lerpReverse) {
			t -= deltaTime;

			if (t <= 0) {
				t = 0;
				beginLerp = false;
				//if (_body != nullptr)
				//	_body->SetType(RigidBodyType::Static);
				if(doUpdateNbors)
					GetGameObject()->GetScene()->pathManager->Get<pathfindingManager>()->UpdateNbors();
			}

		}
		else {
			t += deltaTime;

			if (t >= tLength) {
				t = tLength;
				beginLerp = false;
				//if (_body != nullptr)
				//	_body->SetType(RigidBodyType::Static);
				if (doUpdateNbors)
					GetGameObject()->GetScene()->pathManager->Get<pathfindingManager>()->UpdateNbors();
			}
			 
		}

		GetGameObject()->SetPostion(lerpstuff(glm::vec3(startx, starty, startz), glm::vec3(endx, endy, endz), (t / tLength)));

	}
}

glm::vec3 SlideLerpSystem::lerpstuff(glm::vec3 a, glm::vec3 b, float t) {
	return a * (1 - t) + (b * t);
}