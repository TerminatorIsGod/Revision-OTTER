#include "Gameplay/Components/LerpSystem.h"
#include <GLFW/glfw3.h>
#include "Gameplay/GameObject.h"
#include "Gameplay/Scene.h"
#include "Utils/ImGuiHelper.h"
#include "Gameplay/Components/pathfindingManager.h"
#include "Gameplay/Physics/RigidBody.h"

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

	if (LABEL_LEFT(ImGui::DragFloat3, "End Rotation", &endx, 1.0f)) {

		// Update the editor state with our new values
		guiStore->SetFloat(ImGui::GetID(&endx), endx);
		guiStore->SetFloat(ImGui::GetID(&endy), endy);
		guiStore->SetFloat(ImGui::GetID(&endz), endz);
	}

	LABEL_LEFT(ImGui::DragFloat, "How long", &tLength);

	LABEL_LEFT(ImGui::Checkbox, "Update AI", &doUpdateNbors);

}

nlohmann::json LerpSystem::ToJson() const {
	return {
		{ "startx", startx },
		{ "starty", starty },
		{ "startz", startz },
		{ "endx", endx},
		{ "endy", endy},
		{ "endz",  endz},
		{ "tlength", tLength},
		{ "updateAI", doUpdateNbors}
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
	tLength(1),
	doUpdateNbors(0)
{ }

LerpSystem::~LerpSystem() = default;

LerpSystem::Sptr LerpSystem::FromJson(const nlohmann::json & blob) {
	LerpSystem::Sptr result = std::make_shared<LerpSystem>();
	result->startx = blob["startx"];
	result->starty = blob["starty"];
	result->startz = blob["startz"];
	result->endx = blob["endx"];
	result->endy = blob["endy"];
	result->endz = blob["endz"];
	result->tLength = blob["tlength"];
	result->doUpdateNbors = blob["updateAI"];
	return result;
}

void LerpSystem::Update(float deltaTime) {

	//std::cout << "Is lerping: " << beginLerp << std::endl;
	//std::cout << "T: " << t << " TL: " << tLength << " Direction: " << lerpReverse << std::endl;

	if (beginLerp) {
		Gameplay::Physics::RigidBody::Sptr _body;
		if (GetComponent<Gameplay::Physics::RigidBody>() != NULL)
		{
			_body = GetComponent<Gameplay::Physics::RigidBody>();
			_body->SetType(RigidBodyType::Kinematic);
		}

		if (lerpReverse) {
			t -= deltaTime;

			if (t <= 0) {
				t = 0;
				beginLerp = false;
				if (doUpdateNbors)
				{
					_body->SetType(RigidBodyType::Static);
					GetGameObject()->GetScene()->pathManager->Get<pathfindingManager>()->UpdateNbors();
				}
				//t = 0;
			}
		}
		else {
			t += deltaTime;

			if (t >= tLength) {
				t = tLength;
				beginLerp = false;
				if (doUpdateNbors)
				{
					_body->SetType(RigidBodyType::Static);
					GetGameObject()->GetScene()->pathManager->Get<pathfindingManager>()->UpdateNbors();
				}
				//t = 0;
			}
		}

		//t += deltaTime;

		//std::cout << "StartEular: " << startx << " " << starty << " " << startz << std::endl;
		//std::cout << "EndEular: " << endx << " " << endy << " " << endz << std::endl;

		//std::cout << "Start: " << startRot.x << " " << startRot.y << " " << startRot.z << std::endl;
		//std::cout << "End: " << endRot.x << " " << endRot.y << " " << endRot.z << std::endl;


		//std::cout << "Rotation: " << lerpstuff(glm::vec3(startx, starty, startz), glm::vec3(endx, endy, endz), (t / tLength)).x << " " << lerpstuff(glm::vec3(startx, starty, startz), glm::vec3(endx, endy, endz), (t / tLength)).y << " " << lerpstuff(glm::vec3(startx, starty, startz), glm::vec3(endx, endy, endz), (t / tLength)).z << std::endl;

		GetGameObject()->SetRotation(lerpstuff(glm::vec3(startx, starty, startz), glm::vec3(endx, endy, endz), (t / tLength)));

		//btTransform trans = _body->GetTransform();
		//trans.setIdentity();
		//btQuaternion quat;
		//btScalar btx = GetGameObject()->GetRotation().x;
		//btScalar bty = GetGameObject()->GetRotation().y;
		//btScalar btz = GetGameObject()->GetRotation().z;
		//quat.setEuler(btx, bty, btz);
		//trans.setRotation(quat);

		//if (lerpReverse) {
		//	GetGameObject()->SetRotation(glm::slerp(endRot, startRot, t/tLength));
		//}
		//else {
		//	GetGameObject()->SetRotation(glm::slerp(startRot, endRot, t/tLength));
		//}

	}
}

glm::vec3 LerpSystem::lerpstuff(glm::vec3 a, glm::vec3 b, float t) {
	return a * (1 - t) + (b * t);
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
