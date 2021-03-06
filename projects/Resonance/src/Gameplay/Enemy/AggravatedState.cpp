#include "Gameplay/Enemy/AggravatedState.h"
#include "Gameplay/Scene.h"
#include "Gameplay/Components/SoundEmmiter.h"
#include "Utils\GlmBulletConversions.h"
#include "Gameplay/Enemy/PatrollingState.h"
#include "Gameplay/Components/SimpleCameraControl.h"


EnemyState& AggravatedState::getInstance()
{
	static AggravatedState singleton;
	return singleton;
}

void AggravatedState::Start(Enemy* e)
{
	std::cout << "\n[Enemy] " << e->GetGameObject()->Name << ": Entered Aggravated State";

	e->pathRequested = false;
	e->agroTimer = agroTimerMax;
	e->maxVelocity = e->AgroVelocity;
}

void AggravatedState::End(Enemy* e)
{
	std::cout << "\n[Enemy] " << e->GetGameObject()->Name << ": Exited Aggravated State";
}

void AggravatedState::Listen(Enemy* e, float deltaTime)
{
	if (e->nIndex > 0)
		e->listeningRadius = glm::mix(e->listeningRadius, e->agroStationaryListeningRadius, 2.0f * deltaTime);
	else
		e->listeningRadius = glm::mix(e->listeningRadius, e->agroMovingListeningRadius, 2.0f * deltaTime);

	e->scene->Lights[e->soundLight].Color = glm::mix(e->scene->Lights[e->soundLight].Color, e->red, 8.0f * deltaTime);


	//std::cout << "\n\nAGRO TIMER: " << e->agroTimer;
	if (e->agroTimer > 0)
		e->agroTimer -= 1.0f * deltaTime;
	else
		e->SetState(PatrollingState::getInstance());


	//Checking if any sounds are in listening Radius
	glm::vec3 dir = e->player->GetPosition() - e->GetGameObject()->GetPosition();
	float dist = glm::length(dir);
	float totalRadius = e->player->Get<SoundEmmiter>()->volume + e->listeningRadius;

	if (dist >= totalRadius)
		return;

	if (dist < 7.0f) //Kill player
	{
		e->player->Get<SimpleCameraControl>()->ShowGameOver();
		e->GetGameObject()->GetScene()->requestSceneReload = true;
		e->scene->IsPlaying = false;
	}
	//e->player->SetPostion(e->player->Get<SimpleCameraControl>()->startingPos);

	//Raycasting toward heard sound to determine state change
	btCollisionWorld::ClosestRayResultCallback hit(ToBt(e->GetGameObject()->GetPosition()), ToBt(e->player->GetPosition()));
	e->scene->GetPhysicsWorld()->rayTest(ToBt(e->GetGameObject()->GetPosition()), ToBt(e->player->GetPosition()), hit);

	if (!hit.hasHit())
		return;

	if (hit.m_collisionObject->isStaticObject())
		return;

	//std::cout << "\nMADE IT BRU";

	std::cout << "\nIM AGRO AGAIN!!";
	e->agroTimer = agroTimerMax;

	e->pathRequested = false;
}

void AggravatedState::SwitchIndex(Enemy* e, float deltaTime)
{
	if (e->nIndex > 0)
		e->nIndex--;
}


void AggravatedState::Pathfind(Enemy* e, float deltaTime)
{
	glm::vec3 enemyPos = e->GetGameObject()->GetPosition();
	glm::vec3 soundPos = e->player->GetPosition();

	btCollisionWorld::ClosestRayResultCallback hit(ToBt(enemyPos), ToBt(soundPos));
	e->scene->GetPhysicsWorld()->rayTest(ToBt(enemyPos), ToBt(soundPos), hit);

	if (!hit.hasHit())
		return;

	glm::vec3 objectPos = ToGlm(hit.m_collisionObject->getWorldTransform().getOrigin());
	if (objectPos == e->player->GetPosition())
	{
		e->target = soundPos;
		return;
	}

	if (!e->pathRequested)
	{
		e->pathSet.clear();
		//std::cout << "\nCalculated Path to: " << patrolPos.x << ", " << patrolPos.y << ", " << patrolPos.z;
		e->pathSet = e->pathManager->Get<pathfindingManager>()->requestPath(enemyPos, e->player->GetPosition());

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
		SwitchIndex(e, deltaTime);
	}


}

void AggravatedState::Move(Enemy* e, float  deltaTime)
{
	e->MoveChase(deltaTime);
}

