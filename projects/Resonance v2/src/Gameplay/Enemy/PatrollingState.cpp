#include "Gameplay/Enemy/PatrollingState.h"
#include "Gameplay/Scene.h"
#include "Gameplay/Components/SoundEmmiter.h"
#include "Utils\GlmBulletConversions.h"
#include "Gameplay/Enemy/AggravatedState.h"
#include "Gameplay/Enemy/DistractedState.h"
#include "Gameplay/Components/AudioManager.h"

EnemyState& PatrollingState::getInstance()
{
	static PatrollingState singleton;
	return singleton;
}

void PatrollingState::Start(Enemy* e)
{
	std::cout << "\n[Enemy] " << e->GetGameObject()->Name << ": Entered Patrolling State";
	e->myChannel = e->scene->audioManager->Get<AudioManager>()->PlaySoundByName("LeaflingPatrol", 2.0f, e->GetGameObject()->GetPosition(), true);
	//e->myChannel->addDSP(FMOD_CHANNELCONTROL_DSP_HEAD, e->myDSP);

	e->pathRequested = false;
	e->maxVelocity = e->IdleVelocity;
}

void PatrollingState::End(Enemy* e)
{
	e->myChannel->stop(FMOD_STUDIO_STOP_IMMEDIATE);
	std::cout << "\n[Enemy] " << e->GetGameObject()->Name << ": Exited Patrolling State";
}

void PatrollingState::Listen(Enemy* e, float deltaTime)
{
	//Sound Light Lerping
	e->listeningRadius = glm::mix(e->listeningRadius, e->patrolListeningRadius, 2.0f * deltaTime);
	e->scene->Lights[e->soundLight].Color = glm::mix(e->scene->Lights[e->soundLight].Color, e->blue, 4.0f * deltaTime);

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

		////Determining State Change
		//if (s->Get<SoundEmmiter>()->isPlayerLight)
		//	e->SetState(AggravatedState::getInstance());
		//else
		e->SetState(DistractedState::getInstance());

	}
}

void SwitchIndex(Enemy* e)
{
	if (e->nIndex > 0)
	{
		e->nIndex--;
	}
	else
	{
		if (e->pIndex < e->patrolPoints.size() - 1)
			e->pIndex++;
		else
			e->pIndex = 0;

		e->pathRequested = false;
	}
}

void PatrollingState::Pathfind(Enemy* e, float deltaTime)
{
	//If the enemy has no patrol points, just stay where you are.
	if (e->patrolPoints.size() < 1)
	{
		e->target = e->startPos;
		return;
	}

	//If the enemy has a direct line of sight to a patrol point, 
	// just steer toward it directly, no need for pathfinding
	glm::vec3 enemyPos = e->GetGameObject()->GetPosition();
	glm::vec3 patrolPos = e->patrolPoints[e->pIndex];

	if (glm::length(patrolPos - enemyPos) > 0)
	{
		btCollisionWorld::ClosestRayResultCallback hit(ToBt(enemyPos), ToBt(patrolPos));
		e->scene->GetPhysicsWorld()->rayTest(ToBt(enemyPos), ToBt(patrolPos), hit);

		if (!hit.hasHit())
		{
			e->target = patrolPos;
			if (glm::length(patrolPos - enemyPos) < 3.0f)
				SwitchIndex(e);
			return;
		}
	}

	//Request a path, and set indexes
	if (!e->pathRequested)
	{
		e->pathSet.clear();
		std::cout << "\n[Enemy] " << e->GetGameObject()->Name << " Calculated Path to Patrol Point " << e->pIndex << " (" << patrolPos.x << ", " << patrolPos.y << ", " << patrolPos.z << ")";
		e->pathSet = e->pathManager->Get<pathfindingManager>()->requestPath(enemyPos, patrolPos);

		//This if statement runs if a path could not be found
		if (e->pathSet[0] == glm::vec3(69420.0f, 69420.0f, 69420.0f))
		{
			//Hmm I wonder what should happen if the enemy can't find a path while patrolling? 
			//This isn't something that should ever happen in this state, or level though.
			//It should prob just switch index and go towards the next patrol point
		}

		e->nIndex = e->pathSet.size() - 1;
		e->pathRequested = true;
	}

	e->target = e->pathSet[e->nIndex];

	if (glm::length(e->GetGameObject()->GetPosition() - e->pathSet[e->nIndex]) < 3.f) //3
		SwitchIndex(e);
}

void PatrollingState::Move(Enemy* e, float  deltaTime)
{
	if (e->patrolPoints.size() < 1)
		return;
	e->Move(deltaTime);
}

