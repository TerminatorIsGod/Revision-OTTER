#pragma once
#include "IComponent.h"
#include "Gameplay/GameObject.h"

using namespace Gameplay;

/// <summary>
/// Showcases a very simple behaviour that rotates the parent gameobject at a fixed rate over time
/// </summary>
class ObjectTag : public Gameplay::IComponent {
public:
	typedef std::shared_ptr<ObjectTag> Sptr;

	ObjectTag() = default;

	bool distracter = false;
	bool player = false;
	bool enemy = false;
	bool wall = false;
	bool obstacle = false;

	glm::vec3 speed = glm::vec3(0.0f);
	virtual void RenderImGui() override;
	virtual nlohmann::json ToJson() const override;
	static ObjectTag::Sptr FromJson(const nlohmann::json& data);

	MAKE_TYPENAME(ObjectTag);


};
