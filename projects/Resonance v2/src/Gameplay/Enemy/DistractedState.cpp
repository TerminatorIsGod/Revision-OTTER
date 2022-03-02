#include "Gameplay/Enemy/DistractedState.h"
#include "Gameplay/Scene.h"
#include "Gameplay/Components/SoundEmmiter.h"
#include "Utils\GlmBulletConversions.h"
#include "Gameplay/Enemy/PatrollingState.h"
#include "Gameplay/Enemy/AggravatedState.h"
#include "Gameplay/Enemy/AggravatedState.h"
#include "Gameplay/Components/SimpleCameraControl.h"


EnemyState& DistractedState::getInstance()
{
	static DistractedState singleton;
	return singleton;
}

void DistractedState::Start(Enemy* e)
{
	std::cout << "\n[Enemy] " << e->GetGameObject()->Name << ": Entered Distracted State";
	e->myChannel = e->scene->audioManager->Get<AudioManager>()->PlaySoundByName("LeaflingDistracted", 4.0f, e->GetGameObject()->GetPosition());
	e->pathRequested = false;
	e->maxVelocity = e->IdleVelocity;
	e->distractedBackupTimer = distractedBackupTimerMax;
	e->distractedTimer = distractedTimerMax;
}

void DistractedState::End(Enemy* e)
{
	e->myChannel->stop();
	std::cout << "\n[Enemy] " << e->GetGameObject()->Name << ": Exited Distracted State";
}

void DistractedState::Listen(Enemy* e, float deltaTime)
{
	//Sound Light Lerping
	e->listeningRadius = glm::mix(e->listeningRadius, e->patrolListeningRadius, 2.0f * deltaTime);
	e->scene->Lights[e->soundLight].Color = glm::mix(e->scene->Lights[e->soundLight].Color, e->yellow, 4.0f * deltaTime);

	//std::cout << "\nSWITCHING BACK IN: " << e->distractedTimer;
	//Backup distraction timer
	if (e->distractedBackupTimer > 0)
		e->distractedBackupTimer -= deltaTime;
	else
		e->SetState(PatrollingState::getInstance());

	//Distraction timer when enemy reaches destination
	if (e->lastHeardSounds.size() == 0)
		e->distractedTimer -= deltaTime;

	if (e->distractedTimer <= 0)
		e->SetState(PatrollingState::getInstance());

	for each (GameObject * s in e->scene->soundEmmiters)
	{
		//Checking if any sounds are in listening Radius
		glm::vec3 dir = s->GetPosition() - e->GetGameObject()->GetPosition();
		float dist = glm::length(dir);
		float totalRadius = s->Get<SoundEmmiter>()->volume + e->listeningRadius;

		if (dist >= totalRadius)
			continue;

		//Raycasting toward heard sound to determine state change
		btCollisionWorld::ClosestRayResultCallback hit(ToBt(e->GetGameObject()->GetPosition()), ToBt(s->GetPosition()));
		e->scene->GetPhysicsWorld()->rayTest(ToBt(e->GetGameObject()->GetPosition()), ToBt(s->GetPosition()), hit);

		if (hit.hasHit() && hit.m_collisionObject->isStaticObject())
			continue;

		//Adding the heard sound to our lists (removing them if already there)
		for (int i = 0; i < e->lastHeardSounds.size(); i++)
		{
			if (e->lastHeardSounds[i] == s)
			{
				e->lastHeardPositions.erase(e->lastHeardPositions.begin() + i);
				e->lastHeardSounds.erase(e->lastHeardSounds.begin() + i);
			}
		}

		e->lastHeardSounds.insert(e->lastHeardSounds.begin(), s);

		if (!s->Get<SoundEmmiter>()->isPlayerLight)
			e->lastHeardPositions.insert(e->lastHeardPositions.begin(), s->GetPosition());
		else
			e->lastHeardPositions.insert(e->lastHeardPositions.begin(), e->player->GetPosition()); //If player's sound, go directly to them instead of their sound


		e->pathRequested = false;

		//Switch to agro when super close to player? Turned this off for now, 
		//I find it interesting for the enemies to slowly follow you while you hold ur breath.
		if (glm::length(e->player->GetPosition() - e->GetGameObject()->GetPosition()) < 7.0f)
		{
			//std::cout << "\nIM AGRO!!";
			//e->SetState(AggravatedState::getInstance());
		}

		if (s->Get<SoundEmmiter>()->isPlayerLight)
		{
			btCollisionWorld::ClosestRayResultCallback hit2(ToBt(e->GetGameObject()->GetPosition()), ToBt(e->player->GetPosition()));
			e->scene->GetPhysicsWorld()->rayTest(ToBt(e->GetGameObject()->GetPosition()), ToBt(e->player->GetPosition()), hit2);

			glm::vec3 objectPos = ToGlm(hit2.m_collisionObject->getWorldTransform().getOrigin());
			if (glm::round(objectPos) == glm::round(e->player->GetPosition()))
			{
				//std::cout << "\nIM AGRO!!";
				e->SetState(AggravatedState::getInstance());
			}
		}
	}
}

void DistractedState::SwitchIndex(Enemy* e, float deltaTime)
{
	if (e->nIndex > 0)
		e->nIndex--;
	else
	{
		if (e->lastHeardSounds.size() <= 0)
			return;

		e->lastHeardPositions.erase(e->lastHeardPositions.begin());
		e->lastHeardSounds.erase(e->lastHeardSounds.begin());
		e->pathRequested = false;
	}
}

void DistractedState::Pathfind(Enemy* e, float deltaTime)
{
	if (e->lastHeardSounds.size() <= 0)
		return;

	glm::vec3 enemyPos = e->GetGameObject()->GetPosition();
	glm::vec3 soundPos = e->lastHeardPositions[0];

	btCollisionWorld::ClosestRayResultCallback hit(ToBt(enemyPos), ToBt(soundPos));
	e->scene->GetPhysicsWorld()->rayTest(ToBt(enemyPos), ToBt(soundPos), hit);

	if (!hit.hasHit())
		return;

	glm::vec3 objectPos = ToGlm(hit.m_collisionObject->getWorldTransform().getOrigin());
	if (objectPos == e->lastHeardPositions[0])
	{
		e->target = soundPos;
		if (glm::length(soundPos - enemyPos) < 12.0f)
			SwitchIndex(e, deltaTime);
		return;
	}

	if (!e->pathRequested)
	{
		e->pathSet.clear();
		//std::cout << "\nCalculated Path to: " << patrolPos.x << ", " << patrolPos.y << ", " << patrolPos.z;
		e->pathSet = e->pathManager->Get<pathfindingManager>()->requestPath(enemyPos, e->lastHeardPositions[0]);

		if (e->pathSet[0] == glm::vec3(69420.0f, 69420.0f, 69420.0f))
		{
			e->SetState(PatrollingState::getInstance());
			return;
		}

		e->nIndex = e->pathSet.size() - 1;
		e->pathRequested = true;
	}

	e->target = e->pathSet[e->nIndex];

	if (glm::length(e->GetGameObject()->GetPosition() - e->pathSet[e->nIndex]) < 12.0f)
	{
		SwitchIndex(e, deltaTime);
	}
}

void DistractedState::Move(Enemy* e, float  deltaTime)
{
	e->Move(deltaTime);
}

