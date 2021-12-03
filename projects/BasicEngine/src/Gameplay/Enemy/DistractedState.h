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
	const float distractedBackupTimerMax = 20.0f;
	const float distractedTimerMax = 2.5f;

private:
	//Singleton Stuff
	DistractedState() {}
	DistractedState(const DistractedState& other);
	DistractedState& operator=(const DistractedState& other);

};