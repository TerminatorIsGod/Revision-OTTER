#pragma once
#include "IComponent.h"
#include "Gameplay/GameObject.h"

using namespace Gameplay;

/// <summary>
/// Showcases a very simple behaviour that rotates the parent gameobject at a fixed rate over time
/// </summary>
class pathfindingManager : public IComponent {
public:

	typedef std::shared_ptr<pathfindingManager> Sptr;
	pathfindingManager() = default;
	~pathfindingManager();

	//Properties
	std::vector<GameObject::Sptr> navNodes;
	std::vector<GameObject::Sptr> openSet, closedSet, pathSet;
	GameObject::Sptr startNode, endNode;

	glm::vec3 speed = glm::vec3(0.0f);
	float nborRange = 10.0f;
	Scene* scene;
	int cIndex = 0;
	bool calculatingPath = false;

	//Pathfinding Functions
	void resetGrid();
	void UpdateNbors();
	bool StartAndEndCheck();
	void RunPathfind(); //just merge the ClearPathCalculations() and pathfinding operations
	void CompareOpen();
	float g(GameObject::Sptr n, GameObject::Sptr p);
	void h(GameObject::Sptr n1);
	void f(GameObject::Sptr n1);
	void CalculateFCosts(int nbor);
	void CheckIfEnd(int nbor);
	void OpenNbors(int nbor);
	void SequencePath();
	std::vector<GameObject::Sptr> requestPath(glm::vec3 startPos, glm::vec3 targetPos);
	void CalculatePath();


	//General Functions
	virtual void Awake() override;
	//virtual void Update(float deltaTime) override;


	virtual void RenderImGui() override;

	virtual nlohmann::json ToJson() const override;
	static pathfindingManager::Sptr FromJson(const nlohmann::json& data);

	MAKE_TYPENAME(pathfindingManager);
};