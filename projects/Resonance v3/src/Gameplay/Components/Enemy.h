#pragma once
#include "IComponent.h"
#include "Gameplay/GameObject.h"
#include "Gameplay/Components/pathfindingManager.h"
#include "Gameplay/Physics/RigidBody.h"
#include "Gameplay/Scene.h"
#include "Gameplay/Enemy/EnemyState.h"
#include "fmod.hpp"
#include "Gameplay/Components/AudioManager.h"
#include "Gameplay/Components/Light.h"

class EnemyState; //This forward declaration is here to avoid a circular include

struct GLFWwindow;

using namespace Gameplay;

class Enemy : public IComponent {
public:

	typedef std::shared_ptr<Enemy> Sptr;
	Enemy();
	//~Enemy();

#pragma region "Properties & Variables"

	std::vector<GameObject*> lastHeardSounds;
	std::vector<glm::vec3> lastHeardPositions;
	GameObject::Sptr player;
	Scene* scene;
	GLFWwindow* window;
	Gameplay::Physics::RigidBody::Sptr body;
	glm::quat currentRot;

	glm::vec3 startPos = glm::vec3(0);
	glm::vec3 target;
	float soundExpireTimerDefault = 5.0f;
	float soundExpireTimer;

	//Steering Movement
	float maxVelocity = 4.0f;

	float maxRotationSpeed = 0.1f;
	glm::vec3 desiredVelocity;
	glm::vec3 targetRotation;
	float avoidanceRange = 2.5f; //2.5 is good
	float avoidanceStrength = 1000.0f; //1000 is good, 750 seemed to increase odds of enemies getting stuck

	bool canSeePlayer = false;

	//Listening Light
	float listeningRadius = 3.0f;
	Light::Sptr soundLight;

	//Pathfinding
	bool pathRequested = false;
	GameObject* pathManager;

	std::vector<glm::vec3> patrolPoints;
	int pIndex = 0;
	std::vector<glm::vec3> pathSet;
	int nIndex = 0;

	//State Machine Stuff
	glm::vec3 red = glm::vec3(0.2f, 0, 0);
	glm::vec3 blue = glm::vec3(0.0f, 0.0f, 0.2f);
	glm::vec3 yellow = glm::vec3(0.2f, 0.2f, 0);

	const float agroMovingListeningRadius = 12.0f;
	const float agroStationaryListeningRadius = 6.0f;
	const float patrolListeningRadius = 4.0f; // this is normmally 4

	float AgroVelocity = 13.0f;
	float IdleVelocity = 6.0f;

	float agroTimer = 5.0f;
	float distractedBackupTimer = 20.0f, distractedTimer = 5.0f; //dont use this

	EnemyState* currentState;

	FMOD::Studio::EventInstance* myChannel;
	FMOD::DSP* myDSP;


#pragma endregion "Properties & Variables"

	//Functions
	void SetState(EnemyState& newState);
	inline EnemyState* getCurrentState() const { return currentState; }

	void MoveListeningLight();
	void Move(float deltaTime);
	void MoveChase(float deltaTime);
	void MoveLinear(float deltaTime);
	void Steering(float deltaTime);
	void Chase(float deltaTime);
	void ChaseLinear(float deltaTime);
	void AvoidanceReflect(glm::vec3 dir, float deltaTime);
	void Avoidance(glm::vec3 dir, float deltaTime);
	void IsPlayerDead();

	//General Functions
	glm::vec3 speed = glm::vec3(0.0f);
	bool started = false;
	virtual void Awake() override;
	virtual void Update(float deltaTime) override;
	virtual void RenderImGui() override;
	virtual nlohmann::json ToJson() const override;
	static Enemy::Sptr FromJson(const nlohmann::json& data);

	MAKE_TYPENAME(Enemy);
};