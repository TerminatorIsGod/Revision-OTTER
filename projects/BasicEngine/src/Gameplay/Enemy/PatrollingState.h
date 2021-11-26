#pragma once
#include "Gameplay/Enemy/EnemyState.h"
#include "Gameplay/Components/Enemy.h"

class PatrollingState : public EnemyState
{
public:

	void Start(Enemy* enemy);
	void End(Enemy* enemy);

	void Listen(Enemy* enemy, float deltaTime);
	void Pathfind(Enemy* enemy, float deltaTime);
	void Move(Enemy* enemy, float deltaTime);

	static EnemyState& getInstance();

private:
	//Singleton Stuff
	PatrollingState() {}
	PatrollingState(const PatrollingState& other);
	PatrollingState& operator=(const PatrollingState& other);

};

