#pragma once
#include "Gameplay/Enemy/EnemyState.h"
#include "Gameplay/Components/Enemy.h"

class PatrollingState : public EnemyState
{
public:

	void Start(Enemy* e);
	void End(Enemy* e);

	void Listen(Enemy* e, float deltaTime);
	void Pathfind(Enemy* e, float deltaTime);
	void Move(Enemy* e, float deltaTime);

	static EnemyState& getInstance();

private:
	//Singleton Stuff
	PatrollingState() {}
	PatrollingState(const PatrollingState& other);
	PatrollingState& operator=(const PatrollingState& other);

};

