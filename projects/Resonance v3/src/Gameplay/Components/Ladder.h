#pragma once
#include "IComponent.h"
#include "Gameplay/GameObject.h"
#include "Gameplay/Scene.h"
struct GLFWwindow;

using namespace Gameplay;

/// <summary>
/// Showcases a very simple behaviour that rotates the parent gameobject at a fixed rate over time
/// </summary>
class Ladder : public Gameplay::IComponent {
public:
	typedef std::shared_ptr<Ladder> Sptr;

	Ladder() = default;
	virtual void Awake() override;
	virtual void Update(float deltaTime) override;

	glm::vec3 teleportPos = glm::vec3(0.0f);
	virtual void RenderImGui() override;
	virtual nlohmann::json ToJson() const override;
	static Ladder::Sptr FromJson(const nlohmann::json& data);
	Scene* _scene;
	GLFWwindow* _window;
	bool toLevel2 = false;
	bool isEPressed = false;
	MAKE_TYPENAME(Ladder);

};
