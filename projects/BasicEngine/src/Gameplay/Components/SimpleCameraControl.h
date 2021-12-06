#pragma once
#include "IComponent.h"
#include "Gameplay/Physics/RigidBody.h"
#include "Gameplay/Components/SoundEmmiter.h"

struct GLFWwindow;

/// <summary>
/// A simple behaviour that allows movement of a gameobject with WASD, mouse,
/// and ctrl + space
/// </summary>
class SimpleCameraControl : public Gameplay::IComponent {
public:
	typedef std::shared_ptr<SimpleCameraControl> Sptr;

	SimpleCameraControl();
	virtual ~SimpleCameraControl();

	virtual void Awake() override;
	virtual void Update(float deltaTime) override;

public:
	virtual void RenderImGui() override;
	MAKE_TYPENAME(SimpleCameraControl);
	virtual nlohmann::json ToJson() const override;
	static SimpleCameraControl::Sptr FromJson(const nlohmann::json& blob);

	void Movement(float deltaTime);
	void OxygenSystem(float deltaTime);
	void SwitchState(float deltaTime);
	void Interact(float deltaTime);
	void ShowInteract();

	void IdleState(float deltaTime);
	void SneakState(float deltaTime);
	void WalkState(float deltaTime);
	void RunState(float deltaTime);

	void SetSpeed(float newSpeed);
	void MoveUI(float deltaTime);
protected:
	float _shiftMultipler;
	glm::vec2 _mouseSensitivity;
	glm::vec3 _moveSpeeds;
	glm::dvec2 _prevMousePos;
	glm::vec2 _currentRot;
	glm::quat currentRot;

	bool _allowMouse = false;
	bool isJPressed = false;
	bool isEPressed = false;
	bool freecam = false;
	bool _isMousePressed = false;
	GLFWwindow* _window;
	Scene* _scene;
	//Player State Stuff
	enum PlayerState
	{
		Idle,
		Sneak,
		Walk,
		Run
	};

	PlayerState playerState = Idle;

	float sneakSpeed = 4.0f;
	float walkSpeed = 6.0f;
	float runSpeed = 8.0f;
	float speedScale = 500.0f;

	float idleTimerDefault = 2.0f;
	float idleTimer = idleTimerDefault;
	bool inhale = false;

	//Oxygen Stuff
	float chokeVol = 12.0f;
	float replenishVol = 10.0f;
	float oxygenMeterMax = 120.0f;
	float oxygenMeter = oxygenMeterMax;
	float oxygenDecaySpeed = 1.0f;
	float oxygenReplenishSpeed = 4.0f;
	float breathHoldDecaySpeed = 2.0f;

	SoundEmmiter::Sptr soundEmmiter;
};