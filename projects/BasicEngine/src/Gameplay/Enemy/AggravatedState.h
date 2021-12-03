#pragma once
#include "Gameplay/Enemy/EnemyState.h"
#include "Gameplay/Components/Enemy.h"

class AggravatedState : public EnemyState
{
public:

	void Start(Enemy* e);
	void End(Enemy* e);

	void Listen(Enemy* e, float deltaTime);
	void Pathfind(Enemy* e, float deltaTime);
	void Move(Enemy* e, float deltaTime);

	static EnemyState& getInstance();
	const float agroTimerMax = 8.0f;

private:
	//Singleton Stuff
	AggravatedState() {}
	AggravatedState(const AggravatedState& other);
	AggravatedState& operator=(const AggravatedState& other);

};