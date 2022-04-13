
#include "Gameplay/Components/pathfindingManager.h"
#include "Utils/ImGuiHelper.h"
#include "Utils/JsonGlmHelpers.h"
#include <GLFW/glfw3.h>
#include "Gameplay/Scene.h"
#include "Gameplay/Components/NavNode.h"
#include "Utils\GlmBulletConversions.h"
#include "Application/Application.h"
#include<chrono>

#pragma region "Default Functions"
void pathfindingManager::Awake() {


}

pathfindingManager::~pathfindingManager()
{
	navNodes.clear();
	openSet.clear();
	closedSet.clear();
	pathSet.clear();
	startNode = nullptr;
	endNode = nullptr;
}

void pathfindingManager::Update(float deltaTime)
{
	if (!ran)
	{
		navNodes = GetGameObject()->GetScene()->navNodes;
		Application& app = Application::Get();
		_window = app.GetWindow();
		scene = GetGameObject()->GetScene();
		UpdateNbors();

		//GameObject::Sptr myself = std::make_shared<GameObject>(*GetGameObject());
		scene->pathManager = GetGameObject();
		ran = true;
	}

	//if (glfwGetKey(_window, GLFW_KEY_P))
	//{
	//	float minDistance = 999999;
	//	GameObject* minNode = navNodes[0];
	//	float distance;

	//	//Find Start Node
	//	for (int i = 0; i < navNodes.size(); i++)
	//	{
	//		distance = glm::length(navNodes[i]->GetWorldPosition() - scene->MainCamera->GetGameObject()->GetParent()->GetPosition());
	//		if (distance < minDistance)
	//		{
	//			minDistance = distance;
	//			minNode = navNodes[i];
	//		}
	//	}
	//	startNode = minNode;
	//	std::cout << "\nCLOSEST NODE: " << minNode->Name;
	//}
}


void pathfindingManager::RenderImGui() {
	LABEL_LEFT(ImGui::DragFloat3, "Speed2", &speed.x);
}

nlohmann::json pathfindingManager::ToJson() const {
	return {
		{ "speed", speed }
	};
}

pathfindingManager::Sptr pathfindingManager::FromJson(const nlohmann::json& blob) {
	pathfindingManager::Sptr result = std::make_shared<pathfindingManager>();
	result->speed = JsonGet(blob, "speed", result->speed);

	return result;
}

#pragma endregion "Default Functions"

float SquareMagnitude(glm::vec3 dir)
{
	float dirLength = (dir.x * dir.x) + (dir.y * dir.y) + (dir.z * dir.z);
	return dirLength;
}


void pathfindingManager::resetGrid()
{
	for (int i = 0; i < navNodes.size(); i++)
	{
		navNodes[i]->Get<NavNode>()->Reset();
	}
}

void pathfindingManager::UpdateNbors()
{
	std::cout << "\n\nGRAPH SIZE: " << navNodes.size();
	for (int i = 0; i < navNodes.size(); i++)
	{
		navNodes[i]->Get<NavNode>()->neighbors.clear();

		for (int x = 0; x < navNodes.size(); x++)
		{
			if (navNodes[x] == navNodes[i])
				continue;

			glm::vec3 dir = navNodes[x]->GetWorldPosition() - navNodes[i]->GetWorldPosition();
			float dirLength = glm::sqrt((dir.x * dir.x) + (dir.y * dir.y));

			if (dirLength <= nborRange && dirLength > 0)
			{
				btCollisionWorld::ClosestRayResultCallback hit(ToBt(navNodes[i]->GetWorldPosition()), ToBt(navNodes[x]->GetWorldPosition()));
				scene->GetPhysicsWorld()->rayTest(ToBt(navNodes[i]->GetWorldPosition()), ToBt(navNodes[x]->GetWorldPosition()), hit);

				if (!hit.hasHit())
					navNodes[i]->Get<NavNode>()->neighbors.push_back(navNodes[x]);
			}
		}
	}
	std::cout << "\n\nNavNode Neighbors Updated.";

	//int testNborCount = 0;
	//for (int i = 0; i < navNodes.size(); i++)
	//{
	//	testNborCount += navNodes[i]->Get<NavNode>()->neighbors.size();
	//}

	//std::cout << "\n\nThe Average Branching Factor Is: " << testNborCount / navNodes.size();
}

bool pathfindingManager::StartAndEndCheck()
{
	if (startNode == NULL || endNode == NULL)
		return false;

	if (startNode == endNode)
		return false;

	return true;
}

float pathfindingManager::g(GameObject* n, GameObject* p)
{
	glm::vec3 dir = n->GetWorldPosition() - p->GetWorldPosition();
	return SquareMagnitude(dir) + p->Get<NavNode>()->gCost;
}

void pathfindingManager::h(GameObject* n1)
{
	glm::vec3 dir = n1->GetWorldPosition() - endNode->GetWorldPosition();
	n1->Get<NavNode>()->hCost = SquareMagnitude(dir);
}

void pathfindingManager::f(GameObject* n1)
{
	NavNode::Sptr node = n1->Get<NavNode>();
	node->fCost = node->gCost + node->hCost;
}

void pathfindingManager::CompareOpen()
{
	NavNode::Sptr minNode;
	minNode = openSet[0]->Get<NavNode>();
	int minIndex = 0;

	//Find minimum node in openSet
	for (int i = 0; i < openSet.size(); i++)
	{
		NavNode::Sptr openNode = openSet[i]->Get<NavNode>();
		if (openNode->fCost < minNode->fCost && openNode->hCost < minNode->hCost)
		{
			minNode = openNode;
			minIndex = i;
		}
	}

	//Remove node from openSet
	minNode->NodeType = minNode->closed;
	closedSet.push_back(openSet[minIndex]);
	openSet.erase(openSet.begin() + minIndex);
}

void pathfindingManager::RunPathfind()
{
	if (StartAndEndCheck())
	{
		//Resetting Stuff
		cIndex = 0;
		resetGrid();
		closedSet.clear();
		openSet.clear();
		pathSet.clear();

		//Initializing Pathfind
		closedSet.push_back(startNode);
		calculatingPath = true;

		//start timer
		//auto start = std::chrono::high_resolution_clock::now();
		while (calculatingPath)
		{
			CalculatePath();
		}
		//end timer
		//auto stop = std::chrono::high_resolution_clock::now();
		//auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
		//std::cout << "\nPathfinding Calculation Lasted: " << duration.count() << " micro seconds\n\n";
	}
	else
	{
		std::cout << "\n\nStart & End Point have not been defined!";
	}

}

void pathfindingManager::CalculateFCosts(int nbor)
{
	//Variables storing indexs for the navNodes list

	GameObject* nborObj = closedSet[cIndex]->Get<NavNode>()->neighbors[nbor];

	if (nborObj->Get<NavNode>()->gCost <= 0 || g(nborObj, closedSet[cIndex]) < nborObj->Get<NavNode>()->gCost)
	{
		//Parenting
		nborObj->Get<NavNode>()->parent = closedSet[cIndex];

		//Calculating Costs
		nborObj->Get<NavNode>()->gCost = g(nborObj, closedSet[cIndex]);
		h(nborObj);
		f(nborObj);
	}
}

void pathfindingManager::CheckIfEnd(int nbor)
{
	GameObject* nborObj = closedSet[cIndex]->Get<NavNode>()->neighbors[nbor];

	if (nborObj == endNode)
	{
		nborObj->Get<NavNode>()->parent = closedSet[cIndex];
		SequencePath();
		return;
	}
}

void pathfindingManager::OpenNbors(int nbor)
{
	GameObject* nborObj = closedSet[cIndex]->Get<NavNode>()->neighbors[nbor];

	if (nborObj->Get<NavNode>()->NodeType == nborObj->Get<NavNode>()->blank)
	{
		nborObj->Get<NavNode>()->NodeType = nborObj->Get<NavNode>()->open;
		openSet.push_back(nborObj);
	}
}

void pathfindingManager::SequencePath()
{
	pathSet.push_back(endNode->GetWorldPosition());

	GameObject* current = endNode;
	current = current->Get<NavNode>()->parent;

	while (true)
	{
		if (current == startNode)
		{
			//std::cout << "\nPath Length (In Nodes):  " << pathSet.size();
			calculatingPath = false;
			return;
		}
		else
		{
			pathSet.push_back(current->GetWorldPosition());
			current = current->Get<NavNode>()->parent;
		}
	}
}

std::vector<glm::vec3> pathfindingManager::requestPath(glm::vec3 startPos, glm::vec3 targetPos)
{
	float minDistance = 999999;
	GameObject* minNode = navNodes[0];
	float distance;
	pathSet.clear();

	//Find Start Node
	for (int i = 0; i < navNodes.size(); i++)
	{
		distance = glm::sqrt(SquareMagnitude(navNodes[i]->GetWorldPosition() - startPos));
		if (distance < minDistance)
		{
			minDistance = distance;
			minNode = navNodes[i];
		}
	}
	startNode = minNode;

	minDistance = 999999;
	//Find End Node
	for (int i = 0; i < navNodes.size(); i++)
	{
		distance = glm::sqrt(SquareMagnitude(navNodes[i]->GetWorldPosition() - targetPos));
		if (navNodes[i] == startNode)
			continue;

		if (distance < minDistance)
		{
			minDistance = distance;
			minNode = navNodes[i];
		}
	}
	endNode = minNode;

	RunPathfind();
	if (pathSet.size() > 0)
		return pathSet;
	else //Returns a weird position to signify no path could be found
	{
		pathSet.push_back(glm::vec3(69420.0f, 69420.0f, 69420.0f));
		return pathSet;
	}
}

void pathfindingManager::CalculatePath()
{
	//start timer
	//auto start = std::chrono::high_resolution_clock::now();


	for (int nbor = 0; nbor < closedSet[cIndex]->Get<NavNode>()->neighbors.size(); nbor++)
	{
		//std::cout << "\n\n# NEIGHBORS OF NODE " << closedSet[cIndex]->Name << ": " << nbor;
		CheckIfEnd(nbor);
		OpenNbors(nbor);
		CalculateFCosts(nbor);
	}

	if (openSet.size() <= 0 && cIndex > 0)
	{
		std::cout << "\n\nNo possible path could be found";
		calculatingPath = false;
		return;
	}

	CompareOpen();

	cIndex++;

	//end timer
	//auto stop = std::chrono::high_resolution_clock::now();
	//auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
	//std::cout << "\nPathfinding Step " << cIndex << " Lasted: " << duration.count() << " micro seconds";
}
