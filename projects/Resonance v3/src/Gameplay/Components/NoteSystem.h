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
class NoteSystem : public Gameplay::IComponent {
public:
	typedef std::shared_ptr<NoteSystem> Sptr;

	NoteSystem();
	virtual ~NoteSystem();

	virtual void Awake() override;
	virtual void Update(float deltaTime) override;

	void interact();

public:
	virtual void RenderImGui() override;
	MAKE_TYPENAME(NoteSystem);
	virtual nlohmann::json ToJson() const override;
	static NoteSystem::Sptr FromJson(const nlohmann::json& blob);
	Gameplay::GameObject::Sptr _player;

	LerpSystem::Sptr _lerpS;

	float _distance = 0;
	float _interactDistance = 0;

	std::string noteName{ "" };

	bool isOpen = false;

	bool isKeyPressed = false;
	bool isTerminal = false;
	GLFWwindow* _window;

protected:

	//int _keys;


};