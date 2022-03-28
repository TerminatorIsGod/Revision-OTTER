
#include "Gameplay/Components/pathfindingManager.h"
#include "Utils/ImGuiHelper.h"
#include "Utils/JsonGlmHelpers.h"
#include <GLFW/glfw3.h>
#include "Gameplay/Scene.h"
#include "Gameplay/Components/NavNode.h"
#include "Utils\GlmBulletConversions.h"
#include "Application/Application.h"

#pragma region "Default Functions"
void pathfindingManager::Awake() {

	navNodes = GetGameObject()->GetScene()->navNodes;
	Application& app = Application::Get();
	_window = app.GetWindow();
	scene = GetGameObject()->GetScene();
	UpdateNbors();

	//GameObject::Sptr myself = std::make_shared<GameObject>(*GetGameObject());
	scene->pathManager = GetGameObject();
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
//Pathfinding Stress-test
void pathfindingManager::Update(float deltaTime)
{
	//if (glfwGetKey(_window, GLFW_KEY_P))
	//{
	//	std::vector<GameObject::Sptr> yo = requestPath(glm::vec3(0, 50.0f, 100), scene->MainCamera->GetGameObject()->GetPosition());

	//	for (int i = 0; i < navNodes.size(); i++)
	//	{
	//		navNodes[i]->SetScale(glm::vec3(1.0f));
	//		navNodes[i]->SetRotation(glm::vec3(0.0f));

	//	}

	//	for (int i = 0; i < yo.size(); i++)
	//	{
	//		yo[i]->SetScale(glm::vec3(2.0f, 2.0f, 5.0f));
	//	}

	//	endNode->SetRotation(glm::vec3(90, 0, 0));
	//	startNode->SetRotation(glm::vec3(30, 0, 0));

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
	for (int i = 0; i < navNodes.size(); i++)
	{
		navNodes[i]->Get<NavNode>()->neighbors.clear();

		for (int x = 0; x < navNodes.size(); x++)
		{
			if (navNodes[x] == navNodes[i])
				continue;

			glm::vec3 dir = navNodes[x]->GetPosition() - navNodes[i]->GetPosition();
			float dirLength = glm::sqrt((dir.x * dir.x) + (dir.y * dir.y));

			if (dirLength <= nborRange && dirLength > 0)
			{
				btCollisionWorld::ClosestRayResultCallback hit(ToBt(navNodes[i]->GetPosition()), ToBt(navNodes[x]->GetPosition()));
				scene->GetPhysicsWorld()->rayTest(ToBt(navNodes[i]->GetPosition()), ToBt(navNodes[x]->GetPosition()), hit);

				if (!hit.hasHit())
					navNodes[i]->Get<NavNode>()->neighbors.push_back(navNodes[x]);
			}
		}
	}
	std::cout << "\n\nNavNode Neighbors Updated.";
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
	glm::vec3 dir = n->GetPosition() - p->GetPosition();
	return SquareMagnitude(dir) + p->Get<NavNode>()->gCost;
}

void pathfindingManager::h(GameObject* n1)
{
	glm::vec3 dir = n1->GetPosition() - endNode->GetPosition();
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

		while (calculatingPath)
		{
			CalculatePath();
		}
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
	pathSet.push_back(endNode->GetPosition());

	GameObject* current = endNode;
	current = current->Get<NavNode>()->parent;

	while (true)
	{
		if (current == startNode)
		{
			calculatingPath = false;
			return;
		}
		else
		{
			pathSet.push_back(current->GetPosition());
			current = current->Get<NavNode>()->parent;;
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
		distance = glm::sqrt(SquareMagnitude(navNodes[i]->GetPosition() - startPos));
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
		distance = glm::sqrt(SquareMagnitude(navNodes[i]->GetPosition() - targetPos));
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
	for (int nbor = 0; nbor < closedSet[cIndex]->Get<NavNode>()->neighbors.size(); nbor++)
	{
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

}
