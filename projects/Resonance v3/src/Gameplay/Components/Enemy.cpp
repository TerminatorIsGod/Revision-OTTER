#include "Gameplay/Components/Enemy.h"
#include "Utils/ImGuiHelper.h"
#include "Utils/JsonGlmHelpers.h"
#include "Utils\GlmBulletConversions.h"
#include <GLFW/glfw3.h>
#include "Gameplay/Enemy/PatrollingState.h"
#include "Gameplay/Enemy/AggravatedState.h"
#include "fmod_studio.hpp"
#include "Gameplay/Components/SoundEmmiter.h"


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
	body->SetAngularFactor(glm::vec3(0, 0, 1));
	body->SetLinearVelocity(glm::vec3(0));
	body->SetAngularDamping(0.25f);
	//body->SetLinearDamping(0.0f);
	GetGameObject()->SetPostion(startPos);

	if (isSiren)
	{
		IdleVelocity = 4.0f;
		AgroVelocity = 6.0f;
		agroMovingListeningRadius = 12.0f;
		agroStationaryListeningRadius = 12.0f;
		distractedListeningRadius = 11.0f;
		patrolListeningRadius = 8.0f; // this is normmally 4
		GetComponent<SoundEmmiter>()->isSirenSound = true;
	}

	//Light Stuff
	GameObject::Sptr light = scene->CreateGameObject("Sound Light");
	light->isGenerated = true;
	soundLight = light->Add<Light>();
	soundLight->SetRadius(-listeningRadius * 8.0f);
	soundLight->SetColor(blue);

	player = scene->MainCamera->GetGameObject()->GetParent();
	pathManager = scene->pathManager;

	patrolLists.push_back(&patrolPoints);
	patrolLists.push_back(&patrolPoints2);


}

void Enemy::Update(float deltaTime)
{
	if (!started)
	{
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

		FMOD_3D_ATTRIBUTES attributes;
		attributes.position = aMan->GlmVectorToFmodVector(GetGameObject()->GetPosition());
		attributes.velocity = aMan->GlmVectorToFmodVector(body->GetLinearVelocity());
		myChannel->set3DAttributes(&attributes);

		//Sound Occlusion Rayscast
		btCollisionWorld::ClosestRayResultCallback hit2(ToBt(GetGameObject()->GetPosition() + glm::vec3(0.0f, 0.0f, 2.0f)), ToBt(player->GetPosition()));
		scene->GetPhysicsWorld()->rayTest(ToBt(GetGameObject()->GetPosition() + glm::vec3(0.0f, 0.0f, 2.0f)), ToBt(player->GetPosition()), hit2);

		canSeePlayer = false;

		if (hit2.m_collisionObject == NULL)
			return;

		glm::vec3 objectPos = ToGlm(hit2.m_collisionObject->getWorldTransform().getOrigin());

		float currentWetVal;
		myChannel->getParameterByName("Lowpass Mix", &currentWetVal);

		//Hmm maybe make it so the dampening isn't as intense on non-static objects, than on static objects
		if (hit2.m_collisionObject->isStaticObject())
		{
			currentWetVal = glm::mix(currentWetVal, 1.0f, 1.8f * deltaTime);
			myChannel->setParameterByName("Lowpass Mix", currentWetVal);

			if (scene->isGeneratorOn && pListIndex == 0 && currentWetVal > 0.9f)
			{
				pListIndex = 1;
				GetGameObject()->SetPostion(startPos2);
				pIndex = 0;
				pathRequested = false;
				SetState(PatrollingState::getInstance());
			}
		}
		else if (glm::round(objectPos) == glm::round(player->GetPosition()))
		{
			currentWetVal = glm::mix(currentWetVal, 0.0f, 1.8f * deltaTime);
			myChannel->setParameterByName("Lowpass Mix", currentWetVal);

			canSeePlayer = true;
		}
		else
		{
			btCollisionWorld::ClosestRayResultCallback hit2(ToBt(player->GetPosition()), ToBt(GetGameObject()->GetPosition()));
			scene->GetPhysicsWorld()->rayTest(ToBt(player->GetPosition()), ToBt(GetGameObject()->GetPosition()), hit2);

			if (hit2.m_collisionObject == NULL || hit2.m_collisionObject->isStaticObject())
				return;

			currentWetVal = glm::mix(currentWetVal, 0.0f, 1.8f * deltaTime);
			myChannel->setParameterByName("Lowpass Mix", currentWetVal);
		}
	}


}

void Enemy::RenderImGui() {

	ImGui::DragFloat3("Starting Position Pre-Generator", &startPos.x);
	ImGui::DragFloat3("Starting Position Post-Generator", &startPos2.x);

	std::string blank = " ";
	ImGui::Text(blank.c_str());
	ImGui::Text(("Patrol List #" + std::to_string(pListIndex)).c_str());

	if (ImGui::Button("[+] Add Patrol Point"))
	{
		patrolLists[pListIndex]->push_back(glm::vec3(0));
	}

	if (ImGui::Button("[+] Add Patrol Point (At My Position)"))
	{
		patrolLists[pListIndex]->push_back(player->GetPosition());
	}

	if (ImGui::Button("[-] Remove Patrol Point"))
	{
		if (pIndex != patrolLists[pListIndex]->size() - 1)
			patrolLists[pListIndex]->pop_back();
		else
			std::cout << "\n[Enemy]: Can't Delete, Enemy Currently Moving To that Point!";
	}

	for (int i = 0; i < patrolLists[pListIndex]->size(); i++)
	{
		std::string name1 = "Patrol Points " + std::to_string(i);
		const char* name2 = name1.c_str();
		ImGui::DragFloat3(name2, &((*patrolLists[pListIndex])[i].x));
	}

	ImGui::Text(blank.c_str());
	ImGui::Text(("Using Patrol List #" + std::to_string(pListIndex)).c_str());

	if (ImGui::Button("[Swap Patrol Point Lists]"))
	{
		if (pListIndex == 0)
		{
			pListIndex = 1;
			GetGameObject()->SetPostion(startPos2);
		}
		else
		{
			pListIndex = 0;
			GetGameObject()->SetPostion(startPos);
		}

		pIndex = 0;
		pathRequested = false;
	}

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

	for (int i = 0; i < patrolPoints2.size(); i++)
	{
		result["Patrol2Point" + std::to_string(i)] = patrolPoints2[i];
	}
	result["PatrolPointCount2"] = glm::vec3(patrolPoints2.size());
	result["StartingPosition2"] = startPos2;
	result["IsSiren"] = isSiren;
	return result;
}

Enemy::Sptr Enemy::FromJson(const nlohmann::json& blob) {
	Enemy::Sptr result = std::make_shared<Enemy>();
	for (int i = 0; i < JsonGet(blob, "PatrolPointCount", glm::vec3(result->patrolPoints.size())).x; i++)
	{
		result->patrolPoints.push_back(JsonGet(blob, "PatrolPoint" + std::to_string(i), glm::vec3(0.0f)));
	}
	result->startPos = JsonGet(blob, "StartingPosition", result->startPos);

	for (int i = 0; i < JsonGet(blob, "PatrolPointCount2", glm::vec3(result->patrolPoints2.size())).x; i++)
	{
		result->patrolPoints2.push_back(JsonGet(blob, "Patrol2Point" + std::to_string(i), glm::vec3(0.0f)));
	}
	result->startPos2 = JsonGet(blob, "StartingPosition2", result->startPos2);
	result->isSiren = JsonGet(blob, "IsSiren", result->isSiren);
	return result;
}
#pragma endregion "Default Functions"

void Enemy::MoveListeningLight()
{
	soundLight->GetGameObject()->SetPostion(GetGameObject()->GetPosition());
	soundLight->SetRadius(listeningRadius * listeningRadius * -1.20f);
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

void Enemy::MoveLinear(float deltaTime)
{
	ChaseLinear(deltaTime);

	//AvoidanceReflect(body->GetLinearVelocity(), deltaTime);

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

void Enemy::ChaseLinear(float deltaTime)
{
	glm::vec3 newVel = body->GetLinearVelocity();

	//Steering
	newVel = (player->GetPosition() - GetGameObject()->GetPosition()) * deltaTime;
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
	const glm::vec3 startPoint = GetGameObject()->GetPosition() + glm::vec3(0.0f, 0.0f, 2.0f);
	const glm::vec3 endPoint = startPoint + (dir * avoidanceRange);
	btCollisionWorld::ClosestRayResultCallback hit(ToBt(startPoint), ToBt(endPoint));
	scene->GetPhysicsWorld()->rayTest(ToBt(startPoint), ToBt(endPoint), hit);

	if (!hit.hasHit())
		return;

	//Make sure enemy doesn't avoid player or sound emmiters
	glm::vec3 objectPos = ToGlm(hit.m_collisionObject->getWorldTransform().getOrigin());

	for (int i = 0; i < scene->soundEmmiters.size(); i++)
	{
		if (glm::round(objectPos) == glm::round(scene->soundEmmiters[i]->GetPosition()))
			return;
		if (glm::round(objectPos) == glm::round(player->GetPosition()))
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
	const glm::vec3 startPoint = GetGameObject()->GetPosition() + glm::vec3(0.0f, 0.0f, 2.0f);
	const glm::vec3 endPoint = startPoint + (dir * avoidanceRange);

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