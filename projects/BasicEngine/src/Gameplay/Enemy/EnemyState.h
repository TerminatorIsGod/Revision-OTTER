#pragma once
#include "Gameplay/Components/Enemy.h"

class Enemy; //This forward declaration is here to avoid a circular include

class EnemyState
{
public:

	virtual void Start(Enemy* enemy) = 0;
	virtual void End(Enemy* enemy) = 0;

	virtual void Listen(Enemy* enemy, float deltaTime) = 0;
	virtual void Pathfind(Enemy* enemy, float deltaTime) = 0;
	virtual void Move(Enemy* enemy, float deltaTime) = 0;

	virtual ~EnemyState() {}

};
