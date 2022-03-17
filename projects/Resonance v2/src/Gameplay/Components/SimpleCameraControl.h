#pragma once
#include "IComponent.h"
#include "Gameplay/Physics/RigidBody.h"
#include "Gameplay/Components/SoundEmmiter.h"
#include "fmod_studio.hpp"

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
	float baseHeight = 6.0f;

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
	void ShowLocked();
	void ShowDropThrow();
	void ShowGameOver();
	void FadeInBlack(float deltaTime);
	void FadeOutBlack(float deltaTime);
	void ShowBlack();


	bool promptShown = false;
	bool allowInteraction = true;
	void IdleState(float deltaTime);
	void SneakState(float deltaTime);
	void WalkState(float deltaTime);
	void RunState(float deltaTime);

	void SetSpeed(float newSpeed);
	void MoveUI(float deltaTime);
	void PlaceUI(int index, float xSize, float ySize, float xRatio = 1, float xMultiplier = 0, float yRatio = 1, float yMultiplier = 0);

	void LerpHeight(float heightOffset, float deltaTime, float height);
	SoundEmmiter::Sptr GetRecentEmmiter();
	glm::vec3 viewDir;
	glm::quat currentRot;
	glm::vec3 interactionObjectPos;
	bool holdingBreath = false;

protected:
	glm::vec2 centerPos;
	int windx, windy;

	float _shiftMultipler;
	glm::vec2 _mouseSensitivity;
	glm::vec3 _moveSpeeds;
	glm::dvec2 _prevMousePos;
	glm::vec2 _currentRot;

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
	PlayerState prevState = Idle;


	float sneakSpeed = 6.f; //
	float walkSpeed = 8.f; //2
	float runSpeed = 10.f; //3
	float speedScale = 1.5f; //scaled with volume of light //50 //This was 80 before

	float idleTimerDefault = 2.0f;
	float idleTimer = idleTimerDefault;
	bool inhale = false;

	//Oxygen Stuff
	float chokeVol = 15.0f;
	float replenishVol = 12.0f;
	float oxygenMeterMax = 120.0f;
	float oxygenMeter = oxygenMeterMax;
	float oxygenDecaySpeed = 1.0f;
	float oxygenReplenishSpeed = 6.f;
	float breathHoldDecaySpeed = 2.5f;

	std::vector<SoundEmmiter::Sptr> playerEmmiters;
	int playerEmmiterIndex = 0;
	float playerPulseTimer = 0;
	int playerEmmiterCount = 5;
	float soundDelayTimerMax = 0.1f;
	float soundDelayTimer = 0.0f;
	bool startSoundDelay = false;
	//Prompt Textures
	Texture2D::Sptr p_PickUp;
	Texture2D::Sptr p_Climb;
	Texture2D::Sptr p_Close;
	Texture2D::Sptr p_Open;
	Texture2D::Sptr p_Distract;
	Texture2D::Sptr p_Locked;
	Texture2D::Sptr p_DropThrow;
	Texture2D::Sptr blackTex;
	Texture2D::Sptr gameoverTex;



	bool startedRefill = false;
	bool outOfBreath = false;

	FMOD::Studio::EventInstance* oxygenChannel;
	FMOD::Studio::EventInstance* outOfBreathChannel;

};