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
	glm::vec3 startingPos = glm::vec3(2.7, -8, 6);


public:

	virtual void RenderImGui() override;
	MAKE_TYPENAME(SimpleCameraControl);
	virtual nlohmann::json ToJson() const override;
	static SimpleCameraControl::Sptr FromJson(const nlohmann::json& blob);

	void Movement(float deltaTime);
	void OxygenSystem(float deltaTime);
	void SwitchState(float deltaTime);
	void Interact(float deltaTime);
	void ShowOpen();
	void ShowClose();
	void ShowClimb();
	void ShowPickup();
	void ShowDistract();
	void ShowGameOver();

	bool promptShown = false;
	void IdleState(float deltaTime);
	void SneakState(float deltaTime);
	void WalkState(float deltaTime);
	void RunState(float deltaTime);

	void SetSpeed(float newSpeed);
	void MoveUI(float deltaTime);
	void PlaceUI(int index, float xSize, float ySize, float xRatio = 1, float xMultiplier = 0, float yRatio = 1, float yMultiplier = 0);

	SoundEmmiter::Sptr GetRecentEmmiter();

protected:
	glm::vec2 centerPos;
	int windx, windy;

	float _shiftMultipler;
	glm::vec2 _mouseSensitivity;
	glm::vec3 _moveSpeeds;
	glm::dvec2 _prevMousePos;
	glm::vec2 _currentRot;
	glm::quat currentRot;

	bool _allowMouse = true;
	bool isJPressed = false;
	bool isEPressed = false;
	bool freecam = false;
	bool _isMousePressed = true;
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

	float sneakSpeed = 6.f; //
	float walkSpeed = 8.f; //2
	float runSpeed = 10.f; //3
	float speedScale = 80.f; //scaled with volume of light //50

	float idleTimerDefault = 2.0f;
	float idleTimer = idleTimerDefault;
	bool inhale = false;

	//Oxygen Stuff
	float chokeVol = 15.0f;
	float replenishVol = 12.0f;
	float oxygenMeterMax = 120.0f;
	float oxygenMeter = oxygenMeterMax;
	float oxygenDecaySpeed = 1.0f;
	float oxygenReplenishSpeed = 5.f;
	float breathHoldDecaySpeed = 2.5f;

	std::vector<SoundEmmiter::Sptr> playerEmmiters;
	int playerEmmiterIndex = 0;
	float playerPulseTimer = 0;
	int playerEmmiterCount = 3;

	//Prompt Textures
	Texture2D::Sptr p_PickUp = ResourceManager::CreateAsset<Texture2D>("textures/ui/PickupPrompt.png");
	Texture2D::Sptr p_Climb = ResourceManager::CreateAsset<Texture2D>("textures/ui/ClimbPrompt.png");
	Texture2D::Sptr p_Close = ResourceManager::CreateAsset<Texture2D>("textures/ui/ClosePrompt.png");
	Texture2D::Sptr p_Open = ResourceManager::CreateAsset<Texture2D>("textures/ui/OpenPrompt.png");
	Texture2D::Sptr p_Distract = ResourceManager::CreateAsset<Texture2D>("textures/ui/DistractPrompt.png");




};