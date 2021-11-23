#include "Gameplay/Components/LerpSystem.h"
#include <GLFW/glfw3.h>
#include "Gameplay/GameObject.h"
#include "Gameplay/Scene.h"
#include "Utils/ImGuiHelper.h"

void LerpSystem::Awake()
{

}

void LerpSystem::RenderImGui() {
	ImGuiStorage* guiStore = ImGui::GetStateStorage();

	if (LABEL_LEFT(ImGui::DragFloat3, "Start Rotation", &startx, 1.0f)) {

		// Update the editor state with our new values
		guiStore->SetFloat(ImGui::GetID(&startx), startx);
		guiStore->SetFloat(ImGui::GetID(&starty), starty);
		guiStore->SetFloat(ImGui::GetID(&startz), startz);
	}

	if (LABEL_LEFT(ImGui::DragFloat3, "End Rotation", &startx, 1.0f)) {

		// Update the editor state with our new values
		guiStore->SetFloat(ImGui::GetID(&endx), endx);
		guiStore->SetFloat(ImGui::GetID(&endy), endy);
		guiStore->SetFloat(ImGui::GetID(&endz), endz);
	}

	LABEL_LEFT(ImGui::DragFloat, "How long", &tLength);
}

nlohmann::json LerpSystem::ToJson() const {
	return {
		{ "startx", startx },
		{ "starty", starty },
		{ "startz", startz },
		{ "endx", endx},
		{ "endy", endy},
		{ "endz",  endz},
		{ "tlength", tLength}
	};
}

LerpSystem::LerpSystem() :
	IComponent(),
	startx(0),
	starty(0),
	startz(0),
	endx(0),
	endy(0),
	endz(0),
	tLength(1)
{ }

LerpSystem::~LerpSystem() = default;

LerpSystem::Sptr LerpSystem::FromJson(const nlohmann::json& blob) {
	LerpSystem::Sptr result = std::make_shared<LerpSystem>();
	result->startx = blob["startx"];
	result->starty = blob["starty"];
	result->startz = blob["startz"];
	result->endx = blob["endx"];
	result->endy = blob["endy"];
	result->endz = blob["endz"];
	result->tLength = blob["tlength"];
	return result;
}

void LerpSystem::Update(float deltaTime) {
	if (beginLerp) {
		t += deltaTime;

		if (t > tLength) {
			beginLerp = false;
			t = 0;
		}

		if (lerpReverse) {
			GetGameObject()->SetRotation(glm::slerp(endRot, startRot, t/tLength));
		}
		else {
			GetGameObject()->SetRotation(glm::slerp(startRot, endRot, t/tLength));
		}

	}
}

void LerpSystem::setRotationStart() {
	glm::vec3 tempvec3;
	tempvec3.x = startx;
	tempvec3.y = starty;
	tempvec3.z = startz;

	setRotationStart(tempvec3);
}

void LerpSystem::setRotationStart(glm::vec3 xyz) {
	glm::quat x = glm::vec4(glm::vec3(1.0, 0.0, 0.0), xyz.x);
	glm::quat y = glm::vec4(glm::vec3(0.0, 1.0, 0.0), xyz.y);
	glm::quat z = glm::vec4(glm::vec3(0.0, 0.0, 1.0), xyz.z);

	startRot = x * y * z;
}

void LerpSystem::setRotationEnd() {
	glm::vec3 tempvec3;
	tempvec3.x = endx;
	tempvec3.y = endy;
	tempvec3.z = endz;

	setRotationEnd(tempvec3);
}

void LerpSystem::setRotationEnd(glm::vec3 xyz) {
	glm::quat x = glm::vec4(glm::vec3(1.0, 0.0, 0.0), xyz.x);
	glm::quat y = glm::vec4(glm::vec3(0.0, 1.0, 0.0), xyz.y);
	glm::quat z = glm::vec4(glm::vec3(0.0, 0.0, 1.0), xyz.z);

	endRot = x * y * z;
}
