#pragma once
#include "Gameplay/Enemy/EnemyState.h"
#include "Gameplay/Components/Enemy.h"

class DistractedState : public EnemyState
{
public:

	void Start(Enemy* e);
	void End(Enemy* e);

	void Listen(Enemy* e, float deltaTime);
	void Pathfind(Enemy* e, float deltaTime);
	void Move(Enemy* e, float deltaTime);

	void SwitchIndex(Enemy* e, float deltaTime);
	static EnemyState& getInstance();
	const float distractedBackupTimerMax = 15.0f; //20
	const float distractedTimerMax = 4.f; //3

private:
	//Singleton Stuff
	DistractedState() {}
	DistractedState(const DistractedState& other);
	DistractedState& operator=(const DistractedState& other);

};