#pragma once
#include "IComponent.h"
#include "Gameplay/GameObject.h"
struct GLFWwindow;

using namespace Gameplay;

/// <summary>
/// Showcases a very simple behaviour that rotates the parent gameobject at a fixed rate over time
/// </summary>
class pathfindingManager : public IComponent {
public:

	bool ran = false;
	typedef std::shared_ptr<pathfindingManager> Sptr;
	pathfindingManager() = default;
	~pathfindingManager();

	//Properties
	std::vector<GameObject*> navNodes;
	std::vector<GameObject*> openSet;
	std::vector<GameObject*> closedSet;

	std::vector<glm::vec3> pathSet;
	GameObject* startNode;
	GameObject* endNode;

	glm::vec3 speed = glm::vec3(0.0f);
	float nborRange = 15.0f;
	Scene* scene;
	GLFWwindow* _window;

	int cIndex = 0;
	bool calculatingPath = false;

	//Pathfinding Functions
	void resetGrid();
	void UpdateNbors();
	bool StartAndEndCheck();
	void RunPathfind(); //just merge the ClearPathCalculations() and pathfinding operations
	void CompareOpen();
	float g(GameObject* n, GameObject* p);
	void h(GameObject* n1);
	void f(GameObject* n1);
	void CalculateFCosts(int nbor);
	void CheckIfEnd(int nbor);
	void OpenNbors(int nbor);
	void SequencePath();
	std::vector<glm::vec3> requestPath(glm::vec3 startPos, glm::vec3 targetPos);
	void CalculatePath();


	//General Functions
	virtual void Awake() override;
	virtual void Update(float deltaTime) override;


	virtual void RenderImGui() override;

	virtual nlohmann::json ToJson() const override;
	static pathfindingManager::Sptr FromJson(const nlohmann::json& data);

	MAKE_TYPENAME(pathfindingManager);
};