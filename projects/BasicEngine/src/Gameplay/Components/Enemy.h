#pragma once
#include "IComponent.h"
#include "Gameplay/GameObject.h"
#include "Gameplay/Components/pathfindingManager.h"
#include "Gameplay/Physics/RigidBody.h"
#include "Gameplay/Scene.h"
#include "Gameplay/Enemy/EnemyState.h"

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
	GameObject* player;
	Scene* scene;
	GLFWwindow* window;
	Gameplay::Physics::RigidBody::Sptr body;
	glm::quat currentRot;

	glm::vec3 startPos;
	glm::vec3 target;
	float soundExpireTimerDefault = 5.0f;
	float soundExpireTimer;
	float agroTimer = 5.0f;

	//Steering Movement
	float maxVelocity = 4.0f;
	float AgroVelocity = 8.0f;
	float IdleVelocity = 4.0f;
	float maxRotationSpeed = 0.1f;
	glm::vec3 desiredVelocity;
	glm::vec3 targetRotation;
	float avoidanceRange = 3.0f;
	float avoidanceStrength = 2000.0f;

	//Listening Light
	float listeningRadius = 3.0f;
	int soundLight;

	//Pathfinding
	bool pathRequested = false;
	std::vector<glm::vec3> patrolPoints;
	int pIndex = 0;
	GameObject::Sptr pathManager;
	std::vector<glm::vec3> pathSet; // In unity this was a list of nodes, but I'm pretty sure we'll be fine with just positions
	int nIndex;

	//State Machine Stuff
	glm::vec3 red = glm::vec3(0.019f, 0, 0);
	glm::vec3 blue = glm::vec3(0.0f, 0.0f, 0.019f);
	glm::vec3 yellow = glm::vec3(0.2f, 0.2f, 0);
	const float agroMovingListeningRadius = 12.0f;
	const float agroStationaryListeningRadius = 6.0f;

	const float patrolListeningRadius = 4.0f;

	EnemyState* currentState;

#pragma endregion "Properties & Variables"

	//Functions
	void SetState(EnemyState& newState);
	inline EnemyState* getCurrentState() const { return currentState; }

	void MoveListeningLight();
	void Move(float deltaTime);
	void Steering(float deltaTime);
	void AvoidanceReflect(glm::vec3 dir, float deltaTime);
	void Avoidance(glm::vec3 dir, float deltaTime);
	void IsPlayerDead();

	//General Functions
	glm::vec3 speed = glm::vec3(0.0f);

	virtual void Awake() override;
	virtual void Update(float deltaTime) override;
	virtual void RenderImGui() override;
	virtual nlohmann::json ToJson() const override;
	static Enemy::Sptr FromJson(const nlohmann::json& data);

	MAKE_TYPENAME(Enemy);
};