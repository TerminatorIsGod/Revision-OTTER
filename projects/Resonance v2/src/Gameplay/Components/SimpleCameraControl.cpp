#include "Gameplay/Components/SimpleCameraControl.h"
#include <GLFW/glfw3.h>
#define  GLM_SWIZZLE
#include <GLM/gtc/quaternion.hpp>

#include "Gameplay/GameObject.h"
#include "Gameplay/Scene.h"
#include "Utils/JsonGlmHelpers.h" 
#include "Utils/ImGuiHelper.h"
#include "Utils\GlmBulletConversions.h"
#include "Gameplay/Components/Ladder.h"
#include "Gameplay/Components/UIElement.h"
#include "Application/Application.h"
#include "Gameplay/Components/GUI/RectTransform.h"
#include "Gameplay/Components/GUI/GuiPanel.h"
#include "Gameplay/Components/AudioManager.h"


SimpleCameraControl::SimpleCameraControl() :
	IComponent(),
	_mouseSensitivity({ 0.5f, 0.3f }),
	_moveSpeeds(glm::vec3(1600.0f)),
	_shiftMultipler(2.0f),
	_currentRot(glm::vec2(0.0f)),
	_isMousePressed(true)
{ }

SimpleCameraControl::~SimpleCameraControl()
{
	//Destroy Textures
	p_PickUp->~Texture2D();
	p_Climb->~Texture2D();
	p_Close->~Texture2D();
	p_Open->~Texture2D();
	p_Distract->~Texture2D();
}

void SimpleCameraControl::Awake() {
	_scene = GetGameObject()->GetScene();
	Application& app = Application::Get();
	_window = app.GetWindow();
	GetGameObject()->SetPostion(startingPos);

	//Prompt Textures
	if (p_PickUp == nullptr)
	{
		p_PickUp = ResourceManager::CreateAsset<Texture2D>("textures/ui/PickupPrompt.png");
		p_Climb = ResourceManager::CreateAsset<Texture2D>("textures/ui/ClimbPrompt.png");
		p_Close = ResourceManager::CreateAsset<Texture2D>("textures/ui/ClosePrompt.png");
		p_Open = ResourceManager::CreateAsset<Texture2D>("textures/ui/OpenPrompt.png");
		p_Distract = ResourceManager::CreateAsset<Texture2D>("textures/ui/DistractPrompt.png");
	}
	for (int i = 0; i < playerEmmiterCount; i++)
	{
		GameObject::Sptr soundEmmiter = _scene->CreateGameObject("playerEmmiter");
		{
			soundEmmiter->isGenerated = true;

			SoundEmmiter::Sptr emmiter = soundEmmiter->Add<SoundEmmiter>();
			soundEmmiter->Awake();
			emmiter->muteAtZero = true;
			emmiter->isPlayerLight = true;
			emmiter->linearLerp = true;
			emmiter->defaultColour = glm::vec3(0.45, 0.45, 0.45);
			emmiter->soundLightOffset = glm::vec3(0, 0, -6.0f);
			playerEmmiters.push_back(emmiter);
		}
	}
	glfwSetInputMode(_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}

void SimpleCameraControl::Update(float deltaTime)
{

	Movement(deltaTime);
	SwitchState(deltaTime);
	OxygenSystem(deltaTime);
	MoveUI(deltaTime);
	Interact(deltaTime);

	if (playerPulseTimer <= 0.f)
	{
		playerPulseTimer = 1.0f;

		playerEmmiters[playerEmmiterIndex]->isDecaying = false;
		playerEmmiters[playerEmmiterIndex]->MoveToPlayer();

		if (!startedRefill)
			_scene->audioManager->Get<AudioManager>()->PlayFootstepSound(GetGameObject()->GetPosition());

		if (playerEmmiterIndex < playerEmmiterCount - 1)
			playerEmmiterIndex++;
		else
			playerEmmiterIndex = 0;
	}

	prevState = playerState;
	//std::cout << "\nPulse Timer: " << playerPulseTimer;

}

void SimpleCameraControl::Movement(float deltaTime)
{
	auto _body = GetComponent<Gameplay::Physics::RigidBody>();
	/*if (glfwGetMouseButton(_window, 0)) {
	if (_isMousePressed == false) {
		glfwGetCursorPos(_window, &_prevMousePos.x, &_prevMousePos.y);
	}
	_isMousePressed = true;
} else {
	_isMousePressed = false;
}*/

	if (glfwGetKey(_window, GLFW_KEY_M) && _allowMouse == false) {
		_isMousePressed = !_isMousePressed;
		_allowMouse = true;
		//if (_isMousePressed) {
		//	glfwSetInputMode(_window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
		//}
		//else {
		//	glfwSetInputMode(_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		//}

		std::cout << "Chaning mouse thing\n";
	}
	else if (!glfwGetKey(_window, GLFW_KEY_M)) {
		_allowMouse = false;
	}

	//_isMousePressed = true;

	if (_isMousePressed) {
		glm::dvec2 currentMousePos;
		glfwGetCursorPos(_window, &currentMousePos.x, &currentMousePos.y);

		int wsizex, wsizey;

		glfwGetWindowSize(_window, &wsizex, &wsizey);

		float centerx = (wsizex / 2);
		float centery = (wsizey / 2);

		float xoffset = centerx - currentMousePos.x;
		float yoffset = centery - currentMousePos.y;


		glfwSetCursorPos(_window, centerx, centery);


		_currentRot.x += static_cast<float>(xoffset) * _mouseSensitivity.x;  //_currentRot.x += static_cast<float>(currentMousePos.x - _prevMousePos.x) * _mouseSensitivity.x;
		_currentRot.y += static_cast<float>(yoffset) * _mouseSensitivity.y;
		//std::cout << "\nY Rot: " << _currentRot.y;
		if (_currentRot.y > 172)
			_currentRot.y = 172;
		else if (_currentRot.y < 4.5)
			_currentRot.y = 4.5;

		glm::quat rotX = glm::angleAxis(glm::radians(_currentRot.x), glm::vec3(0, 0, 1));
		glm::quat rotY = glm::angleAxis(glm::radians(_currentRot.y), glm::vec3(1, 0, 0));
		currentRot = rotX * rotY;



		GetGameObject()->SetRotation(currentRot);



		_prevMousePos = currentMousePos;

		glm::vec3 input = glm::vec3(0.0f);
		if (glfwGetKey(_window, GLFW_KEY_W)) {
			input.z = -_moveSpeeds.x;
		}
		if (glfwGetKey(_window, GLFW_KEY_S)) {
			input.z = _moveSpeeds.x;
		}
		if (glfwGetKey(_window, GLFW_KEY_A)) {
			input.x = -_moveSpeeds.y;
		}
		if (glfwGetKey(_window, GLFW_KEY_D)) {
			input.x = _moveSpeeds.y;
		}


		if (glfwGetKey(_window, GLFW_KEY_LEFT_SHIFT))
			playerState = Run;
		//else if (glfwGetKey(_window, GLFW_KEY_LEFT_CONTROL))
			//playerState = Sneak;
		else
			playerState = Walk;

		float velocityMagnitude = glm::sqrt((_body->GetLinearVelocity().x * _body->GetLinearVelocity().x) + (_body->GetLinearVelocity().y * _body->GetLinearVelocity().y));

		if (velocityMagnitude < 0.5f)
		{
			playerState = Idle;

		}
		else
		{
			playerEmmiters[playerEmmiterIndex]->lerpSpeed = playerEmmiters[playerEmmiterIndex]->attackSpeed;
		}

		if (glfwGetKey(_window, GLFW_KEY_J))
		{
			if (!isJPressed)
			{
				isJPressed = true;
				if (freecam)
					freecam = false;
				else
					freecam = true;
			}
		}
		else
			isJPressed = false;

		//input *= deltaTime;

		glm::vec3 worldMovement = currentRot * glm::vec4(input, 1.0f);


		if (_body == nullptr) {
			GetGameObject()->SetPostion(GetGameObject()->GetPosition() + worldMovement);
			return;
		}


		_body->SetAngularFactor(glm::vec3(0, 0, 0));

		glm::vec3 physicsMovement = worldMovement;

		if (!freecam)
		{
			physicsMovement.z = 0.0f;//0;
		}

		_body->SetLinearVelocity(glm::vec3(physicsMovement));

		glfwSetInputMode(_window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
	}
	else {
		glfwSetInputMode(_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	}

}

void SimpleCameraControl::OxygenSystem(float deltaTime)
{
	//std::cout << "\nO2: " << oxygenMeter;
	//Fill Oxygen
	glm::vec4 curCol = _scene->uiImages[2]->GetChildren()[0]->Get<GuiPanel>()->GetColor();

	if (glfwGetKey(_window, GLFW_KEY_SPACE) && !glfwGetKey(_window, GLFW_KEY_LEFT_CONTROL))
	{
		if (oxygenMeter < oxygenMeterMax)
		{
			if (!startedRefill)
			{
				oxygenChannel = _scene->audioManager->Get<AudioManager>()->PlaySoundByName("OxygenRefill", 0.6f);
				startedRefill = true;
			}
			oxygenMeter += oxygenReplenishSpeed * deltaTime;
			playerEmmiters[playerEmmiterIndex]->targetVolume = replenishVol;
			playerPulseTimer -= deltaTime * 0.5f;
		}
		else
		{
			oxygenMeter = oxygenMeterMax;
			if (startedRefill)
			{
				oxygenChannel->stop();
				startedRefill = false;
			}
		}

		glm::vec4 newCol = glm::mix(curCol, glm::vec4(0.0f, 0.5f, 1.0f, 0.6f), 3.0f * deltaTime);
		_scene->uiImages[2]->GetChildren()[0]->Get<GuiPanel>()->SetColor(newCol);
	}
	//Oxygen Decay
	else
	{
		if (startedRefill)
		{
			oxygenChannel->stop();
			startedRefill = false;
		}

		if (oxygenMeter > 0.01f)
		{
			oxygenMeter -= oxygenDecaySpeed * deltaTime;
		}
		else
		{
			oxygenMeter = 0.01f;
			playerEmmiters[playerEmmiterIndex]->targetVolume = chokeVol;
		}

		glm::vec4 newCol = glm::mix(curCol, glm::vec4(0.0f, 0.4f, 0.8f, 0.6f), 2.0f * deltaTime);
		_scene->uiImages[2]->GetChildren()[0]->Get<GuiPanel>()->SetColor(newCol);
	}

	if (glfwGetKey(_window, GLFW_KEY_LEFT_CONTROL))//Hold Breath
	{
		SetSpeed(1.0f);

		if (oxygenMeter > 0.01f)
		{
			oxygenMeter -= breathHoldDecaySpeed * deltaTime;
			playerEmmiters[playerEmmiterIndex]->targetVolume = 0.01f;
		}
		else
		{
			oxygenMeter = 0.01f;
		}

		glm::vec4 newCol = glm::mix(curCol, glm::vec4(0.0f, 0.3f, 0.6f, 0.6f), 3.0f * deltaTime);
		_scene->uiImages[2]->GetChildren()[0]->Get<GuiPanel>()->SetColor(newCol);
	}

	//I tested out having the oxygen level affect the sound ring colour, I don't think it's noticible enough to replace the UI.
	//soundEmmiter->colour = soundEmmiter->defaultColour * (oxygenMeter / oxygenMeterMax);
}


void SimpleCameraControl::SwitchState(float deltaTime)
{
	if (playerState != prevState)
		playerPulseTimer = 0.0001f;

	switch (playerState)
	{
	case 0:
		IdleState(deltaTime);
		break;
	case 1:
		SneakState(deltaTime);
		break;
	case 2:
		WalkState(deltaTime);
		break;
	case 3:
		RunState(deltaTime);
		break;
	}
}

void SimpleCameraControl::Interact(float deltaTime)
{

	viewDir = currentRot * glm::vec4(0.0f, 0.0f, -1.0f, 1.0f);
	btCollisionWorld::ClosestRayResultCallback hit(ToBt(GetGameObject()->GetPosition()), ToBt(GetGameObject()->GetPosition() + (viewDir * 5.0f)));
	_scene->GetPhysicsWorld()->rayTest(ToBt(GetGameObject()->GetPosition()), ToBt(GetGameObject()->GetPosition() + (viewDir * 5.0f)), hit);

	if (!hit.hasHit())
		return;

	glm::vec3 objectPos = ToGlm(hit.m_collisionObject->getWorldTransform().getOrigin());
	if (objectPos == glm::vec3(0))
		return;

	//Distraction Items
	for (int i = 0; i < _scene->soundEmmiters.size(); i++)
	{
		if (objectPos != _scene->soundEmmiters[i]->GetPosition())
			continue;

		//UI Prompt
		ShowDistract();

		if (glfwGetKey(_window, GLFW_KEY_E))
		{
			if (!isEPressed)
			{
				_scene->soundEmmiters[i]->Get<SoundEmmiter>()->targetVolume = _scene->soundEmmiters[i]->Get<SoundEmmiter>()->distractionVolume;
				_scene->soundEmmiters[i]->Get<SoundEmmiter>()->isDecaying = false;
				_scene->soundEmmiters[i]->Get<SoundEmmiter>()->lerpSpeed = 4.0f;
				isEPressed = true;
			}
		}
		else
			isEPressed = false;
	}

	//Ladder
	for (int i = 0; i < _scene->ladders.size(); i++)
	{
		if (objectPos != _scene->ladders[i]->GetPosition())
			continue;

		//Ui Prompt
		ShowClimb();

		if (glfwGetKey(_window, GLFW_KEY_E))
		{
			if (!isEPressed)
			{
				GetGameObject()->SetPostion(_scene->ladders[i]->Get<Ladder>()->teleportPos);
				isEPressed = true;
			}
		}
		else
			isEPressed = false;
	}
}

void SimpleCameraControl::ShowOpen()
{
	_scene->uiImages[3]->GetChildren()[0]->Get<GuiPanel>()->SetColor(glm::vec4(1.0f));
	_scene->uiImages[3]->GetChildren()[0]->Get<GuiPanel>()->SetTexture(p_Open);
	promptShown = true;
}

void SimpleCameraControl::ShowClose()
{
	_scene->uiImages[3]->GetChildren()[0]->Get<GuiPanel>()->SetColor(glm::vec4(1.0f));
	_scene->uiImages[3]->GetChildren()[0]->Get<GuiPanel>()->SetTexture(p_Close);
	promptShown = true;
}

void SimpleCameraControl::ShowClimb()
{
	_scene->uiImages[3]->GetChildren()[0]->Get<GuiPanel>()->SetColor(glm::vec4(1.0f));
	_scene->uiImages[3]->GetChildren()[0]->Get<GuiPanel>()->SetTexture(p_Climb);
	promptShown = true;
}

void SimpleCameraControl::ShowPickup()
{
	_scene->uiImages[3]->GetChildren()[0]->Get<GuiPanel>()->SetColor(glm::vec4(1.0f));
	_scene->uiImages[3]->GetChildren()[0]->Get<GuiPanel>()->SetTexture(p_PickUp);
	promptShown = true;
}

void SimpleCameraControl::ShowDistract()
{
	_scene->uiImages[3]->GetChildren()[0]->Get<GuiPanel>()->SetColor(glm::vec4(1.0f));
	_scene->uiImages[3]->GetChildren()[0]->Get<GuiPanel>()->SetTexture(p_Distract);
	promptShown = true;
}

void SimpleCameraControl::ShowGameOver()
{
	_scene->uiImages[4]->GetChildren()[0]->Get<GuiPanel>()->SetColor(glm::vec4(1.0f));
	promptShown = true;
}



void SimpleCameraControl::IdleState(float deltaTime)
{
	SetSpeed(walkSpeed);
	idleTimer -= deltaTime;
	playerPulseTimer -= deltaTime * 0.5f;
	//playerPulseTimer -= deltaTime * 2.0f;

	if (idleTimer <= 0.0f)
	{
		playerEmmiters[playerEmmiterIndex]->lerpSpeed = 0.5f;

		if (inhale)
			inhale = false;
		else
			inhale = true;

		idleTimer = idleTimerDefault;
	}

	if (inhale)
		playerEmmiters[playerEmmiterIndex]->targetVolume = 5.f;
	else
		playerEmmiters[playerEmmiterIndex]->targetVolume = 6.f;
}

void SimpleCameraControl::SneakState(float deltaTime)
{
	SetSpeed(sneakSpeed);
	playerEmmiters[playerEmmiterIndex]->targetVolume = sneakSpeed;
	playerEmmiters[playerEmmiterIndex]->lerpSpeed = 1.0f;
}

void SimpleCameraControl::WalkState(float deltaTime)
{
	SetSpeed(walkSpeed);
	playerPulseTimer -= deltaTime * 2.0f;

	playerEmmiters[playerEmmiterIndex]->targetVolume = walkSpeed;
	playerEmmiters[playerEmmiterIndex]->lerpSpeed = 2.0f;
}

void SimpleCameraControl::RunState(float deltaTime)
{
	SetSpeed(runSpeed);
	playerPulseTimer -= deltaTime * 2.5f;

	playerEmmiters[playerEmmiterIndex]->targetVolume = runSpeed;
	playerEmmiters[playerEmmiterIndex]->lerpSpeed = 2.5f;
}

void SimpleCameraControl::SetSpeed(float newSpeed)
{
	_moveSpeeds = glm::vec3(newSpeed * speedScale);
}

void SimpleCameraControl::MoveUI(float deltaTime)
{
	glfwGetWindowSize(_window, &windx, &windy);
	centerPos.x = windx / 2;
	centerPos.y = windy / 2;

	PlaceUI(0, 10, 10); // Crosshair
	PlaceUI(1, 60, 60, 16, 7, 16, 7); //Oxygen Meter
	PlaceUI(2, 60, 60.0f * (oxygenMeter / oxygenMeterMax), 16, 7, 16, 7); //Oxygen Fill
	PlaceUI(3, 30, 30, 1, 0, 12, 1); // Interaction Prompt
	PlaceUI(4, windx / 4.0f, windy / 4.0f, 1, 0, 2, 1); // Game Over Screen


	if (!promptShown)
		_scene->uiImages[3]->GetChildren()[0]->Get<GuiPanel>()->SetColor(glm::vec4(0.0f));

	promptShown = false;
}

void SimpleCameraControl::PlaceUI(int index, float xSize, float ySize, float xRatio, float xMultiplier, float yRatio, float yMultiplier)
{
	_scene->uiImages[index]->Get<RectTransform>()->SetSize(glm::vec2(0.0f, 0.0f));
	_scene->uiImages[index]->GetChildren()[0]->Get<RectTransform>()->SetSize(glm::vec2(xSize, ySize));
	auto crosshairUI = _scene->uiImages[index]->Get<RectTransform>();
	glm::vec2 uiSize = _scene->uiImages[index]->GetChildren()[0]->Get<RectTransform>()->GetSize();

	glm::vec2 sizeAdj = -glm::vec2(uiSize.x, uiSize.y * 2.0f) / 2.0f;
	glm::vec2 posAdj = glm::vec2((windx / xRatio) * xMultiplier, (windy / yRatio) * yMultiplier);

	crosshairUI->SetPosition(centerPos + sizeAdj + posAdj);
}

SoundEmmiter::Sptr SimpleCameraControl::GetRecentEmmiter()
{
	return playerEmmiters[playerEmmiterIndex];
}



void SimpleCameraControl::RenderImGui()
{
	LABEL_LEFT(ImGui::DragFloat2, "Mouse Sensitivity", &_mouseSensitivity.x, 0.01f);
	LABEL_LEFT(ImGui::DragFloat3, "Move Speed       ", &_moveSpeeds.x, 0.01f, 0.01f);
	LABEL_LEFT(ImGui::DragFloat, "Shift Multiplier ", &_shiftMultipler, 0.01f, 1.0f);
}

nlohmann::json SimpleCameraControl::ToJson() const {
	return {
		{ "mouse_sensitivity", _mouseSensitivity},
		{ "move_speed", _moveSpeeds },
		{ "shift_mult", _shiftMultipler }
	};
}

SimpleCameraControl::Sptr SimpleCameraControl::FromJson(const nlohmann::json& blob) {
	SimpleCameraControl::Sptr result = std::make_shared<SimpleCameraControl>();
	result->_mouseSensitivity = JsonGet(blob, "mouse_sensitivity", result->_mouseSensitivity);
	result->_moveSpeeds = JsonGet(blob, "move_speed", result->_moveSpeeds);
	result->_shiftMultipler = JsonGet(blob, "shift_mult", 2.0f);
	return result;
}
