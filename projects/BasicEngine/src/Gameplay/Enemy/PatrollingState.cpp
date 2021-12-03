#include "Gameplay/Enemy/PatrollingState.h"
#include "Gameplay/Scene.h"
#include "Gameplay/Components/SoundEmmiter.h"
#include "Utils\GlmBulletConversions.h"
#include "Gameplay/Enemy/AggravatedState.h"

EnemyState& PatrollingState::getInstance()
{
	static PatrollingState singleton;
	return singleton;
}

void PatrollingState::Start(Enemy* e)
{
	std::cout << "\n" << e->GetGameObject()->Name << ": Entered Patrolling State";

	e->pathRequested = false;
	e->maxVelocity = e->IdleVelocity;
}

void PatrollingState::End(Enemy* e)
{
	std::cout << "\n" << e->GetGameObject()->Name << ": Exited Patrolling State";
}

void PatrollingState::Listen(Enemy* e, float deltaTime)
{
	for each (GameObject * s in e->scene->soundEmmiters)
	{
		//Checking if any sounds are in listening Radius
		glm::vec3 dir = s->GetPosition() - e->GetGameObject()->GetPosition();
		float dist = glm::length(dir);
		float totalRadius = s->Get<SoundEmmiter>()->volume + e->listeningRadius;

		if (dist >= totalRadius)
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
		e->lastHeardPositions.insert(e->lastHeardPositions.begin(), s->GetPosition());

		//Raycasting toward heard sound to determine state change
		btCollisionWorld::ClosestRayResultCallback hit(ToBt(e->GetGameObject()->GetPosition()), ToBt(s->GetPosition()));
		e->scene->GetPhysicsWorld()->rayTest(ToBt(e->GetGameObject()->GetPosition()), ToBt(s->GetPosition()), hit);

		if (!hit.hasHit())
			continue;

		glm::vec3 objectPos = ToGlm(hit.m_collisionObject->getWorldTransform().getOrigin());

		if (objectPos == e->player->GetPosition())
		{
			std::cout << "\nIM AGRO!!";
			e->SetState(AggravatedState::getInstance());
		}
		else
		{
			std::cout << "\nIM Distracted!!";
			//e->SetState(DistractedState::getInstance());
		}

	}

	e->listeningRadius = glm::mix(e->listeningRadius, e->patrolListeningRadius, 2.0f * deltaTime);
	e->scene->Lights[e->soundLight].Color = glm::mix(e->scene->Lights[e->soundLight].Color, e->blue, 4.0f * deltaTime);

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
	if (e->patrolPoints.size() < 1)
	{
		e->target = e->startPos;
		return;
	}

	glm::vec3 enemyPos = e->GetGameObject()->GetPosition();
	glm::vec3 patrolPos = e->patrolPoints[e->pIndex];

	btCollisionWorld::ClosestRayResultCallback hit(ToBt(enemyPos), ToBt(patrolPos));
	e->scene->GetPhysicsWorld()->rayTest(ToBt(enemyPos), ToBt(patrolPos), hit);

	if (!hit.hasHit())
	{
		e->target = patrolPos;
		if (glm::length(patrolPos - enemyPos) < 3.0f)
			SwitchIndex(e);
		return;
	}

	if (!e->pathRequested)
	{
		e->pathSet.clear();
		std::cout << "\nCalculated Path to: " << patrolPos.x << ", " << patrolPos.y << ", " << patrolPos.z;
		e->pathSet = e->pathManager->Get<pathfindingManager>()->requestPath(enemyPos, patrolPos);
		if (e->pathSet[0] == glm::vec3(69420.0f, 69420.0f, 69420.0f))
		{
			//Hmm I wonder what should happen if the enemy can't find a path while patrolling? 
			//This isn't something that should ever happen in this state tho.
			//Oooh it should prob just switch index and go towards the next patrol point
			//e->pathSet[0] = e->GetGameObject()->GetPosition();
		}

		e->nIndex = e->pathSet.size() - 1;
		e->pathRequested = true;
	}

	e->target = e->pathSet[e->nIndex];

	if (glm::length(e->GetGameObject()->GetPosition() - e->pathSet[e->nIndex]) < 3.0f)
		SwitchIndex(e);



}

void PatrollingState::Move(Enemy* e, float  deltaTime)
{
	if (e->patrolPoints.size() < 1)
		return;
	e->Move(deltaTime);
}

