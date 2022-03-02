#include "Gameplay/Components/Enemy.h"
#include "Utils/ImGuiHelper.h"
#include "Utils/JsonGlmHelpers.h"
#include "Utils\GlmBulletConversions.h"
#include <GLFW/glfw3.h>
#include "Gameplay/Enemy/PatrollingState.h"
#include "Gameplay/Enemy/AggravatedState.h"


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
	body = GetComponent<Gameplay::Physics::RigidBody>();
	body->SetAngularFactor(glm::vec3(0, 0, 0));
	body->SetLinearVelocity(glm::vec3(0));
	body->SetAngularDamping(100.0f);
	//body->SetLinearDamping(0.2f);
	GetGameObject()->SetPostion(startPos);

	scene->Lights.push_back(Light());
	soundLight = scene->Lights.size() - 1;
	scene->Lights[soundLight].isGenerated = true;

	scene->Lights[soundLight].Range = -listeningRadius * 8.0f;;
	scene->Lights[soundLight].Color = blue;

	player = scene->MainCamera->GetGameObject();
	pathManager = scene->pathManager;

}

void Enemy::Update(float deltaTime)
{
	if (!started)
	{
		scene->audioManager->Get<AudioManager>()->system->createDSPByType(FMOD_DSP_TYPE_MULTIBAND_EQ, &myDSP);
		myDSP->setParameterInt(FMOD_DSP_MULTIBAND_EQ_A_FILTER, FMOD_DSP_MULTIBAND_EQ_FILTER_LOWPASS_12DB);
		myDSP->setParameterFloat(FMOD_DSP_MULTIBAND_EQ_A_FREQUENCY, 1000.0f);
		myDSP->setParameterFloat(FMOD_DSP_MULTIBAND_EQ_A_Q, 0.707);

		SetState(PatrollingState::getInstance());
		started = true;
	}



	MoveListeningLight();
	currentState->Listen(this, deltaTime);
	currentState->Pathfind(this, deltaTime);
	currentState->Move(this, deltaTime); //In Agro state, make it so the enemy doesn't slow down when its near target


	if (lastHeardSounds.size() > 2)
	{
		lastHeardSounds.pop_back();
		lastHeardPositions.pop_back();
	}

	GetGameObject()->SetPostion(glm::vec3(GetGameObject()->GetPosition().x, GetGameObject()->GetPosition().y, startPos.z));

	if (myChannel != NULL)
	{
		AudioManager::Sptr aMan = scene->audioManager->Get<AudioManager>();
		myChannel->set3DAttributes(&aMan->GlmVectorToFmodVector(GetGameObject()->GetPosition()), &aMan->GlmVectorToFmodVector(body->GetLinearVelocity()));

		//Sound Occlusion Rayscast
		btCollisionWorld::ClosestRayResultCallback hit2(ToBt(GetGameObject()->GetPosition()), ToBt(player->GetPosition()));
		scene->GetPhysicsWorld()->rayTest(ToBt(GetGameObject()->GetPosition()), ToBt(player->GetPosition()), hit2);

		if (hit2.m_collisionObject == NULL)
			return;

		glm::vec3 objectPos = ToGlm(hit2.m_collisionObject->getWorldTransform().getOrigin());

		float currentWetVal;
		myDSP->getWetDryMix(nullptr, &currentWetVal, nullptr);

		//Hmm maybe make it so the dampening isn't as intense on non-static objects, than on static objects
		if (hit2.m_collisionObject->isStaticObject())
		{
			currentWetVal = glm::mix(currentWetVal, 1.0f, 1.8f * deltaTime);
			myDSP->setWetDryMix(1.0f, currentWetVal, 1.0f - currentWetVal);
		}
		else if (glm::round(objectPos) == glm::round(player->GetPosition()))
		{
			currentWetVal = glm::mix(currentWetVal, 0.0f, 1.8f * deltaTime);
			myDSP->setWetDryMix(1.0f, currentWetVal, 1.0f - currentWetVal);
		}
		else
		{
			btCollisionWorld::ClosestRayResultCallback hit2(ToBt(player->GetPosition()), ToBt(GetGameObject()->GetPosition()));
			scene->GetPhysicsWorld()->rayTest(ToBt(player->GetPosition()), ToBt(GetGameObject()->GetPosition()), hit2);

			if (hit2.m_collisionObject == NULL || hit2.m_collisionObject->isStaticObject())
				return;

			currentWetVal = glm::mix(currentWetVal, 0.0f, 1.8f * deltaTime);
			myDSP->setWetDryMix(1.0f, currentWetVal, 1.0f - currentWetVal);

		}
	}
}

void Enemy::RenderImGui() {
	for (int i = 0; i < patrolPoints.size(); i++)
	{
		std::string name1 = "Patrol Points " + std::to_string(i);
		const char* name2 = name1.c_str();
		ImGui::DragFloat3(name2, &patrolPoints[i].x);
	}

	if (ImGui::Button("[+] Add Patrol Point"))
	{
		patrolPoints.push_back(glm::vec3(0));
	}

	if (ImGui::Button("[+] Add Patrol Point (At My Position)"))
	{
		patrolPoints.push_back(player->GetPosition());
	}

	if (ImGui::Button("[-] Remove Patrol Point"))
	{
		if (pIndex != patrolPoints.size() - 1)
			patrolPoints.pop_back();
		else
			std::cout << "\n[Enemy]: Can't Delete, Enemy Currently Moving To that Point!";
	}

	ImGui::DragFloat3("Starting Position", &startPos.x);
}

nlohmann::json Enemy::ToJson() const {

	nlohmann::json result;
	// Write out RigidBody data
	for (int i = 0; i < patrolPoints.size(); i++)
	{
		result["PatrolPoint" + std::to_string(i)] = patrolPoints[i];
	}
	result["PatrolPointCount"] = glm::vec3(patrolPoints.size());
	result["StartingPosition"] = startPos;
	return result;
}

Enemy::Sptr Enemy::FromJson(const nlohmann::json& blob) {
	Enemy::Sptr result = std::make_shared<Enemy>();
	for (int i = 0; i < JsonGet(blob, "PatrolPointCount", glm::vec3(result->patrolPoints.size())).x; i++)
	{
		result->patrolPoints.push_back(JsonGet(blob, "PatrolPoint" + std::to_string(i), glm::vec3(0.0f)));
	}
	result->startPos = JsonGet(blob, "StartingPosition", result->startPos);

	return result;
}
#pragma endregion "Default Functions"

void Enemy::MoveListeningLight()
{
	scene->Lights[soundLight].Position = GetGameObject()->GetPosition();
	scene->Lights[soundLight].Range = listeningRadius * listeningRadius * -1.20f;
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

	glm::vec3 vel = body->GetLinearVelocity();
	glm::vec3 leftDir = glm::vec3(-vel.y + vel.x, vel.x + vel.y, 0.0f) / 2.0f;
	glm::vec3 rightDir = glm::vec3(vel.y + vel.x, -vel.x + vel.y, 0.0f) / 2.0f;

	Avoidance(leftDir, deltaTime);
	Avoidance(rightDir, deltaTime);
	Avoidance(glm::vec3(-body->GetLinearVelocity().y, body->GetLinearVelocity().x, 0.0f), deltaTime);
	Avoidance(glm::vec3(body->GetLinearVelocity().y, -body->GetLinearVelocity().x, 0.0f), deltaTime);

	GetGameObject()->LookAt(GetGameObject()->GetPosition() + body->GetLinearVelocity() * -1.0f);
}

void Enemy::MoveChase(float deltaTime)
{
	Chase(deltaTime);

	AvoidanceReflect(body->GetLinearVelocity(), deltaTime);

	glm::vec3 vel = body->GetLinearVelocity();
	glm::vec3 leftDir = glm::vec3(-vel.y + vel.x, vel.x + vel.y, 0.0f) / 2.0f;
	glm::vec3 rightDir = glm::vec3(vel.y + vel.x, -vel.x + vel.y, 0.0f) / 2.0f;

	Avoidance(leftDir, deltaTime);
	Avoidance(rightDir, deltaTime);
	Avoidance(glm::vec3(-body->GetLinearVelocity().y, body->GetLinearVelocity().x, 0.0f), deltaTime);
	Avoidance(glm::vec3(body->GetLinearVelocity().y, -body->GetLinearVelocity().x, 0.0f), deltaTime);

	GetGameObject()->LookAt(GetGameObject()->GetPosition() + body->GetLinearVelocity() * -1.0f);
}

void Enemy::Steering(float deltaTime)
{
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

void Enemy::Chase(float deltaTime)
{
	glm::vec3 newVel = body->GetLinearVelocity();

	if (target == glm::vec3(0.0f))
		return;

	//Steering
	desiredVelocity = target - GetGameObject()->GetPosition();
	targetRotation = desiredVelocity - body->GetLinearVelocity();
	if (Magnitude(targetRotation) > maxRotationSpeed)
		targetRotation = (targetRotation / Magnitude(targetRotation));

	//Velocity
	newVel += targetRotation * 100.0f * deltaTime;
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