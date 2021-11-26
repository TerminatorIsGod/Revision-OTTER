#include "Gameplay/Enemy/PatrollingState.h"

EnemyState& PatrollingState::getInstance()
{
	static PatrollingState singleton;
	return singleton;
}

void PatrollingState::Start(Enemy* enemy)
{
	std::cout << "\nEntered Patrolling State";
}

void PatrollingState::End(Enemy* enemy)
{
	std::cout << "\nExited Patrolling State";
}

void PatrollingState::Listen(Enemy* enemy, float deltaTime)
{
}

void PatrollingState::Pathfind(Enemy* enemy, float deltaTime)
{
}

void PatrollingState::Move(Enemy* enemy, float  deltaTime)
{
	enemy->Move(deltaTime);
}
