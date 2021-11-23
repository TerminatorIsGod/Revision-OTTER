#include "Gameplay/Components/Enemy.h"
#include "Utils/ImGuiHelper.h"
#include "Utils/JsonGlmHelpers.h"
#include "Utils\GlmBulletConversions.h"
#include <GLFW/glfw3.h>

#pragma region "Default Functions"

float Magnitude(glm::vec3 dir)
{
	float dirLength = (dir.x * dir.x) + (dir.y * dir.y) + (dir.z * dir.z);
	return glm::sqrt(dirLength);
}

void Enemy::Awake()
{
	scene = GetGameObject()->GetScene();
	window = scene->Window;
	body = GetComponent<Gameplay::Physics::RigidBody>();
	body->SetAngularFactor(glm::vec3(0, 0, 0));

	//scene->Lights.push_back(Light());
	//soundLight = &scene->Lights[scene->Lights.size() - 1];
	//soundLight->Range = -listeningRadius * 16.0f;;
	//soundLight->Color = blue;

}

void Enemy::Update(float deltaTime)
{
	//MoveListeningLight();
	//currentState.Listen();
	//currentState.Pathfind();
	//currentState.Move(); //In Agro state, make it so the enemy doesn't slow down when its near target
	Move(deltaTime);


	if (lastHeardSounds.size() > 2)
	{
		lastHeardSounds.pop_back();
		lastHeardPositions.pop_back();
	}


}

void Enemy::RenderImGui() {
	LABEL_LEFT(ImGui::DragFloat3, "Speed", &speed.x);
}

nlohmann::json Enemy::ToJson() const {
	return {
		{ "speed", GlmToJson(speed) }
		//Eventually make it so it saves the nodes's nbor list. You could do this by saving the index of the node's nbors in the navNodes list.
	};
}

Enemy::Sptr Enemy::FromJson(const nlohmann::json& data) {
	Enemy::Sptr result = std::make_shared<Enemy>();
	result->speed = ParseJsonVec3(data["speed"]);
	return result;
}
#pragma endregion "Default Functions"

void Enemy::MoveListeningLight()
{
	soundLight->Position = GetGameObject()->GetPosition();
}

void Enemy::Move(float deltaTime)
{
	if (glfwGetKey(window, GLFW_KEY_Q))
		Steering(deltaTime);

	//AvoidanceReflect();
	//Avoidance();
	//Avoidance();
	//Avoidance();
	//Avoidance();
}

void Enemy::Steering(float deltaTime)
{
	target = player->GetPosition();

	//glm::vec3 dir = player->GetPosition() - GetGameObject()->GetPosition();
	//body->SetLinearVelocity(dir);
	glm::vec3 newVel = body->GetLinearVelocity();

	if (target == glm::vec3(0.0f))
		return;

	//Steering
	desiredVelocity = target - GetGameObject()->GetPosition();
	targetRotation = desiredVelocity - body->GetLinearVelocity();

	if (Magnitude(targetRotation) > maxRotationSpeed)
		targetRotation = (targetRotation / Magnitude(targetRotation)) * maxRotationSpeed;

	//Velocity
	newVel += targetRotation * 100.0f * deltaTime;

	if (Magnitude(newVel) > maxVelocity)
		newVel = (newVel / Magnitude(newVel)) * maxVelocity;

	body->SetLinearVelocity(glm::vec3(newVel.x, newVel.y, 0.0f));

	GetGameObject()->LookAt(GetGameObject()->GetPosition() + body->GetLinearVelocity() * -1.0f);
}

void Enemy::AvoidanceReflect(glm::vec3 dir)
{

}

void Enemy::Avoidance(glm::vec3 dir)
{

}

void Enemy::IsPlayerDead()
{

}