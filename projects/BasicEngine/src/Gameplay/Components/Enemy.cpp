#include "Gameplay/Components/Enemy.h"
#include "Utils/ImGuiHelper.h"
#include "Utils/JsonGlmHelpers.h"
#include "Utils\GlmBulletConversions.h"
#include <GLFW/glfw3.h>
#include "Gameplay/Enemy/PatrollingState.h"

#pragma region "Default Functions"

float Magnitude(glm::vec3 dir)
{
	float dirLength = (dir.x * dir.x) + (dir.y * dir.y) + (dir.z * dir.z);
	return glm::sqrt(dirLength);
}
Enemy::Enemy()
{
	//Patrol State by default
	currentState = &PatrollingState::getInstance();
}
void Enemy::Awake()
{
	scene = GetGameObject()->GetScene();
	window = scene->Window;
	body = GetComponent<Gameplay::Physics::RigidBody>();
	body->SetAngularFactor(glm::vec3(0, 0, 0));
	body->SetLinearVelocity(glm::vec3(0));
	//body->SetLinearDamping(0.2f);

	scene->Lights.push_back(Light());
	soundLight = scene->Lights.size() - 1;
	scene->Lights[soundLight].Range = -listeningRadius * 8.0f;;
	scene->Lights[soundLight].Color = blue;
	player = scene->MainCamera->GetGameObject();
	SetState(PatrollingState::getInstance());
}

void Enemy::Update(float deltaTime)
{
	MoveListeningLight();
	currentState->Listen(this, deltaTime);
	currentState->Pathfind(this, deltaTime);
	currentState->Move(this, deltaTime); //In Agro state, make it so the enemy doesn't slow down when its near target


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
	scene->Lights[soundLight].Position = GetGameObject()->GetPosition();
}

void Enemy::SetState(EnemyState& newState)
{
	currentState->End(this);
	currentState = &newState;
	currentState->Start(this);
}

void Enemy::Move(float deltaTime)
{
	Steering(deltaTime);

	AvoidanceReflect(body->GetLinearVelocity(), deltaTime);

	glm::vec3 leftDir = glm::vec3(-body->GetLinearVelocity().y + body->GetLinearVelocity().x, body->GetLinearVelocity().x + body->GetLinearVelocity().y, 0.0f) / 2.0f;
	glm::vec3 rightDir = glm::vec3(body->GetLinearVelocity().y + body->GetLinearVelocity().x, -body->GetLinearVelocity().x + body->GetLinearVelocity().y, 0.0f) / 2.0f;

	//Avoidance(leftDir, deltaTime);
	//Avoidance(rightDir, deltaTime);
	Avoidance(glm::vec3(-body->GetLinearVelocity().y, body->GetLinearVelocity().x, 0.0f), deltaTime);
	Avoidance(glm::vec3(body->GetLinearVelocity().y, -body->GetLinearVelocity().x, 0.0f), deltaTime);

	GetGameObject()->LookAt(GetGameObject()->GetPosition() + body->GetLinearVelocity() * -1.0f);
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
		newVel = glm::normalize(newVel) * maxVelocity;

	body->SetLinearVelocity(glm::vec3(newVel.x, newVel.y, 0.0f));

}

void Enemy::AvoidanceReflect(glm::vec3 dir, float deltaTime)
{
	//Check if greater than zero before normalizing since that would divide by 0
	if (Magnitude(dir) <= 0.0f)
		return;

	dir = glm::normalize(dir);

	//Perform Raycast
	const glm::vec3 startPoint = GetGameObject()->GetPosition();
	const glm::vec3 endPoint = GetGameObject()->GetPosition() + (dir * avoidanceRange);

	btCollisionWorld::ClosestRayResultCallback hit(ToBt(startPoint), ToBt(endPoint));
	scene->GetPhysicsWorld()->rayTest(ToBt(startPoint), ToBt(endPoint), hit);

	if (!hit.hasHit())
		return;

	//Make sure enemy doesn't avoid player or sound emmiters
	glm::vec3 objectPos = ToGlm(hit.m_collisionObject->getWorldTransform().getOrigin());

	for (int i = 0; i < scene->soundEmmiters.size(); i++)
	{
		if (objectPos == scene->soundEmmiters[i]->GetPosition())
			return;
	}

	//Add avoidance force
	glm::vec3 newDir = glm::reflect(dir, ToGlm(hit.m_hitNormalWorld));
	newDir = (newDir * avoidanceRange) - GetGameObject()->GetPosition();

	body->ApplyForce(glm::normalize(newDir) * avoidanceStrength * deltaTime);
}

void Enemy::Avoidance(glm::vec3 dir, float deltaTime)
{
	if (Magnitude(dir) <= 0.0f)
		return;

	dir = glm::normalize(dir);

	//Perform Raycast
	const glm::vec3 startPoint = GetGameObject()->GetPosition();
	const glm::vec3 endPoint = GetGameObject()->GetPosition() + (dir * avoidanceRange);

	btCollisionWorld::ClosestRayResultCallback hit(ToBt(startPoint), ToBt(endPoint));
	scene->GetPhysicsWorld()->rayTest(ToBt(startPoint), ToBt(endPoint), hit);

	if (!hit.hasHit())
		return;


	//Add avoidance force
	glm::vec3 newDir = glm::normalize(body->GetLinearVelocity()) - dir;
	newDir = glm::vec3(newDir.x, newDir.y, 0.0f);

	body->ApplyForce(glm::normalize(newDir) * avoidanceStrength * deltaTime);
}

void Enemy::IsPlayerDead()
{

}