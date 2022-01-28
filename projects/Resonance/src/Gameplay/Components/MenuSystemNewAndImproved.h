#pragma once
#include "IComponent.h"
#include "Gameplay/Physics/RigidBody.h"
#include "Gameplay/GameObject.h"
#include "Gameplay/Components/LerpSystem.h"

struct GLFWwindow;

/// <summary>
/// A simple behaviour that applies an impulse along the Z axis to the 
/// rigidbody of the parent when the space key is pressed
/// </summary>
class MenuSystemNewAndImproved : public Gameplay::IComponent {
public:
	typedef std::shared_ptr<MenuSystemNewAndImproved> Sptr;

	MenuSystemNewAndImproved();
	virtual ~MenuSystemNewAndImproved();

	virtual void Awake() override;
	virtual void Update(float deltaTime) override;

	void ToggleMenu();


public:
	virtual void RenderImGui() override;
	MAKE_TYPENAME(MenuSystemNewAndImproved);
	virtual nlohmann::json ToJson() const override;
	static MenuSystemNewAndImproved::Sptr FromJson(const nlohmann::json& blob);

	GLFWwindow* _window;
	Gameplay::Scene* _scene;
	glm::vec2 centerPos;
	glm::vec2 offscreenPos;
	
	int key;

	bool isToggled = false;
	bool isKeyPressed = false;
	
protected:

	//int _keys;

	
};