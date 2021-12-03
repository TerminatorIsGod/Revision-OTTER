#include "Gameplay/Enemy/AggravatedState.h"
#include "Gameplay/Scene.h"
#include "Gameplay/Components/SoundEmmiter.h"
#include "Utils\GlmBulletConversions.h"
#include "Gameplay/Enemy/PatrollingState.h"

EnemyState& AggravatedState::getInstance()
{
	static AggravatedState singleton;
	return singleton;
}

void AggravatedState::Start(Enemy* e)
{
	std::cout << "\n" << e->GetGameObject()->Name << ": Entered Aggravated State";

	e->pathRequested = false;
	e->agroTimer = agroTimerMax;
	e->maxVelocity = e->AgroVelocity;
}

void AggravatedState::End(Enemy* e)
{
	std::cout << "\n" << e->GetGameObject()->Name << ": Exited Aggravated State";
}

void AggravatedState::Listen(Enemy* e, float deltaTime)
{
	//std::cout << "\n\nAGRO TIMER: " << e->agroTimer;
	if (e->agroTimer > 0)
		e->agroTimer -= 1.0f * deltaTime;
	else
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

		if (!hit.hasHit())
			continue;

		glm::vec3 objectPos = ToGlm(hit.m_collisionObject->getWorldTransform().getOrigin());

		if (objectPos == glm::vec3(0))
			return;

		if (objectPos == e->player->GetPosition())
		{
			std::cout << "\nIM AGRO AGAIN!!";
			e->agroTimer = agroTimerMax;

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
			e->pathRequested = false;
		}
	}

	if (e->nIndex > 0)
		e->listeningRadius = glm::mix(e->listeningRadius, e->agroStationaryListeningRadius, 2.0f * deltaTime);
	else
		e->listeningRadius = glm::mix(e->listeningRadius, e->agroMovingListeningRadius, 2.0f * deltaTime);

	e->scene->Lights[e->soundLight].Color = glm::mix(e->scene->Lights[e->soundLight].Color, e->red, 8.0f * deltaTime);

}

void AggravatedState::Pathfind(Enemy* e, float deltaTime)
{

	glm::vec3 enemyPos = e->GetGameObject()->GetPosition();
	glm::vec3 soundPos = e->lastHeardPositions[0];

	btCollisionWorld::ClosestRayResultCallback hit(ToBt(enemyPos), ToBt(soundPos));
	e->scene->GetPhysicsWorld()->rayTest(ToBt(enemyPos), ToBt(soundPos), hit);

	if (!hit.hasHit())
		return;

	glm::vec3 objectPos = ToGlm(hit.m_collisionObject->getWorldTransform().getOrigin());
	if (objectPos == e->player->GetPosition())
	{
		e->target = soundPos;
		//if (glm::length(soundPos - enemyPos) < 3.0f)
		//	SwitchIndex2(e);
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

	if (glm::length(e->GetGameObject()->GetPosition() - e->pathSet[e->nIndex]) < 3.0f)
	{
		if (e->nIndex > 0)
			e->nIndex--;
	}


}

void AggravatedState::Move(Enemy* e, float  deltaTime)
{
	e->Move(deltaTime);
}

