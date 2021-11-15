#pragma once
#include "IComponent.h"
#include "Gameplay/GameObject.h"

using namespace Gameplay;

/// <summary>
/// Showcases a very simple behaviour that rotates the parent gameobject at a fixed rate over time
/// </summary>
class NavNode : public Gameplay::IComponent {
public:
	typedef std::shared_ptr<NavNode> Sptr;

	NavNode() = default;
	~NavNode()
	{
		neighbors.clear();
	}
	//Properties
	std::vector <GameObject::Sptr> neighbors;
	GameObject::Sptr parent = NULL; //Check if parent index > 0 to see if a node actually has a parent or not
	float hCost = 0, gCost = 0, fCost = 0;
	enum type
	{
		blank,
		open,
		closed,
		start,
		end,
		path
	};

	type NodeType = blank;
	void Reset();
	void ClearCosts();

	glm::vec3 speed = glm::vec3(0.0f);

	virtual void Awake() override;

	virtual void RenderImGui() override;

	virtual nlohmann::json ToJson() const override;
	static NavNode::Sptr FromJson(const nlohmann::json& data);

	MAKE_TYPENAME(NavNode);
};