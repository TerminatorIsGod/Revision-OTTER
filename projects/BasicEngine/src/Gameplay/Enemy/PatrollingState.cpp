#include "Gameplay/Enemy/PatrollingState.h"
#include "Gameplay/Scene.h"
#include "Gameplay/Components/SoundEmmiter.h"
#include "Utils\GlmBulletConversions.h"

EnemyState& PatrollingState::getInstance()
{
	static PatrollingState singleton;
	return singleton;
}

void PatrollingState::Start(Enemy* e)
{
	std::cout << "\nEntered Patrolling State";
	e->pathRequested = false;
	e->scene->Lights[e->soundLight].Color = e->blue;
	e->maxVelocity = e->IdleVelocity;
}

void PatrollingState::End(Enemy* e)
{
	std::cout << "\nExited Patrolling State";
}

void PatrollingState::Listen(Enemy* e, float deltaTime)
{
	std::cout << "\n";
	for each (GameObject * s in e->scene->soundEmmiters)
	{
		//Checking if any sounds are in listening Radius
		glm::vec3 dir = s->GetPosition() - e->GetGameObject()->GetPosition();
		float dist = glm::length(dir);
		float totalRadius = s->Get<SoundEmmiter>()->volume + e->listeningRadius;

		//if (s == e->player)
		//	std::cout << "\nDist: " << dist - totalRadius;

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
		e->pathRequested = false;

		//Raycasting toward heard sound to determine state change
		btCollisionWorld::ClosestRayResultCallback hit(ToBt(e->GetGameObject()->GetPosition()), ToBt(s->GetPosition()));
		e->scene->GetPhysicsWorld()->rayTest(ToBt(e->GetGameObject()->GetPosition()), ToBt(s->GetPosition()), hit);

		if (!hit.hasHit())
			continue;

		glm::vec3 objectPos = ToGlm(hit.m_collisionObject->getWorldTransform().getOrigin());

		if (objectPos == glm::vec3(0))
			return;

		if (objectPos == s->GetPosition())
			//e->SetState(AggravatedState::getInstance());
			std::cout << "\nIM AGRO!!";
		else
			//e->SetState(DistractedState::getInstance());
			std::cout << "\nIM Distracted!!";

	}

}

void PatrollingState::Pathfind(Enemy* e, float deltaTime)
{
}

void PatrollingState::Move(Enemy* e, float  deltaTime)
{
	e->Move(deltaTime);
}

