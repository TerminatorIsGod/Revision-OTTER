#pragma once
#include "IComponent.h"
#include "Gameplay/Physics/RigidBody.h"
#include "Gameplay/GameObject.h"
#include "Gameplay/Components/LerpSystem.h"
#include "Gameplay/Components/SlideLerpSystem.h"

struct GLFWwindow;

/// <summary>
/// A simple behaviour that applies an impulse along the Z axis to the 
/// rigidbody of the parent when the space key is pressed
/// </summary>
class InteractSystem2 : public Gameplay::IComponent {
public:
	typedef std::shared_ptr<InteractSystem2> Sptr;

	InteractSystem2();
	virtual ~InteractSystem2();

	virtual void Awake() override;
	virtual void Update(float deltaTime) override;

	void interact();

public:
	virtual void RenderImGui() override;
	MAKE_TYPENAME(InteractSystem2);
	virtual nlohmann::json ToJson() const override;
	static InteractSystem2::Sptr FromJson(const nlohmann::json& blob);
	Gameplay::GameObject::Sptr _player;
	
	LerpSystem::Sptr _lerpS;
	SlideLerpSystem::Sptr _slideLerp;

	float _distance = 0;
	float _interactDistance = 0;

	bool _requiresKey = false;
	bool _iskey = false;
	int _requiredKey = 0;

	bool isOpen = false;

	bool isKeyPressed = false;
	bool _isGenerator = false;
	bool _isDefaultLockedByGenerator = false;
	bool _isLockedAfterGenIsOn = false;
	bool _whenOpenUseDistance = false;

	GLFWwindow* _window;

protected:

	//int _keys;

	
};