#include "Gameplay/Components/SimpleCameraControl.h"
#include <GLFW/glfw3.h>
#define  GLM_SWIZZLE
#include <GLM/gtc/quaternion.hpp>

#include "Gameplay/GameObject.h"
#include "Gameplay/Scene.h"
#include "Utils/JsonGlmHelpers.h" 
#include "Utils/ImGuiHelper.h"
#include "Utils/GlmBulletConversions.h"
#include "Gameplay/Components/Ladder.h"
#include "Gameplay/Components/UIElement.h"
#include "Application/Application.h"
#include "Gameplay/Components/GUI/RectTransform.h"
#include "Gameplay/Components/GUI/GuiPanel.h"
#include "Gameplay/Components/AudioManager.h"
#include "Camera.h"
#include <Gameplay/InputEngine.h>
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
	p_Locked->~Texture2D();
	p_DropThrow->~Texture2D();
	p_Read->~Texture2D();
	p_Activate->~Texture2D();

	blackTex->~Texture2D();
	gameoverTex->~Texture2D();
	loadingTex->~Texture2D();
}

void SimpleCameraControl::Awake() {

}

void SimpleCameraControl::Update(float deltaTime)
{
	if (!updateStarted)
	{
		_scene = GetGameObject()->GetScene();
		Application& app = Application::Get();
		_window = app.GetWindow();

		if (app.scenePath.substr(Application::Get().scenePath.find_last_of("/\\") + 1) == "level2.json")
		{
			if (app.exitedLeft)
				GetGameObject()->SetPostion(glm::vec3(-56.4, 7.0, 6.0));
			else
				GetGameObject()->SetPostion(glm::vec3(56.4, 7.0, 6.0));
		}
		else
		{
			GetGameObject()->SetPostion(startingPos);
		}

		//Prompt Textures
		if (p_PickUp == nullptr)
		{
			p_PickUp = ResourceManager::CreateAsset<Texture2D>("textures/ui/PickupPrompt.png");
			p_Climb = ResourceManager::CreateAsset<Texture2D>("textures/ui/ClimbPrompt.png");
			p_Close = ResourceManager::CreateAsset<Texture2D>("textures/ui/ClosePrompt.png");
			p_Open = ResourceManager::CreateAsset<Texture2D>("textures/ui/OpenPrompt.png");
			p_Distract = ResourceManager::CreateAsset<Texture2D>("textures/ui/DistractPrompt.png");
			p_Locked = ResourceManager::CreateAsset<Texture2D>("textures/ui/LockedPrompt.png");
			p_DropThrow = ResourceManager::CreateAsset<Texture2D>("textures/ui/DropThrow Prompt.png");
			p_Read = ResourceManager::CreateAsset<Texture2D>("textures/ui/Read Prompt.png");
			p_Activate = ResourceManager::CreateAsset<Texture2D>("textures/ui/Activate Prompt.png");
			blackTex = ResourceManager::CreateAsset<Texture2D>("textures/black.png");
			gameoverTex = ResourceManager::CreateAsset<Texture2D>("textures/ui/deathScreen.jpg");
			loadingTex = ResourceManager::CreateAsset<Texture2D>("textures/ui/LoadingScreen.png");

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
				emmiter->defaultColour = glm::vec3(0.1f, 0.0f, 0.45f);
				emmiter->soundLightOffset = glm::vec3(0, 0, -6.0f);
				emmiter->soundName = "";
				playerEmmiters.push_back(emmiter);
			}
			std::cout << "\n\nCreated sound emmiter #" << playerEmmiters.size();
		}
		glfwSetInputMode(_window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);


		ShowBlack();
		_scene->audioManager->Get<AudioManager>()->PlaySoundByName("Gasp", 0.5f);
		_scene->MainCamera->Aperture = 3.0f;
		GetGameObject()->SetRotation(glm::vec3(0.0f));
		updateStarted = true;
	}
	else
	{
		FadeOutBlack(deltaTime);

		viewDir = currentRot * glm::vec4(0.0f, 0.0f, -1.0f, 1.0f);

		Movement(deltaTime);
		SwitchState(deltaTime);
		OxygenSystem(deltaTime);
		MoveUI(deltaTime);

		if (allowInteraction)
			Interact(deltaTime);
		else
			interactionObjectPos = glm::vec3(0.0f);

		EmitSound(deltaTime);
	}


}

void SimpleCameraControl::Movement(float deltaTime)
{
	//if (!Application::Get().IsFocused)
	//	return;


	auto _body = GetComponent<Gameplay::Physics::RigidBody>();

	if (glfwGetKey(_window, GLFW_KEY_M) && _allowMouse == false) {
		_isMousePressed = !_isMousePressed;
		_allowMouse = true;

		std::cout << "Chaning mouse thing\n";
	}
	else if (!glfwGetKey(_window, GLFW_KEY_M)) {
		_allowMouse = false;
	}

	if (_isMousePressed) {

		glfwSetInputMode(_window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

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


		GetGameObject()->GetChildren()[0]->SetRotation(currentRot);

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
				oxygenChannel = _scene->audioManager->Get<AudioManager>()->PlaySoundByName("OxygenRefill", 0.5f);
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
				oxygenChannel->stop(FMOD_STUDIO_STOP_IMMEDIATE);
				_scene->audioManager->Get<AudioManager>()->PlaySoundByName("StopReplenish", 0.5f);
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
			oxygenChannel->stop(FMOD_STUDIO_STOP_IMMEDIATE);
			_scene->audioManager->Get<AudioManager>()->PlaySoundByName("StopReplenish", 0.5f);
			startedRefill = false;
		}

		if (oxygenMeter > 0.01f)
		{
			oxygenMeter -= oxygenDecaySpeed * deltaTime;

			float lerpedDistortion = glm::lerp(_scene->MainCamera->LensDepth, 0.0f, deltaTime);
			_scene->MainCamera->LensDepth = lerpedDistortion;
			if (outOfBreath)
			{
				outOfBreath = false;
				outOfBreathChannel->stop(FMOD_STUDIO_STOP_IMMEDIATE);
			}
		}
		else
		{
			oxygenMeter = 0.01f;
			if (!outOfBreath)
			{
				outOfBreathChannel = _scene->audioManager->Get<AudioManager>()->PlaySoundByName("OutOfBreath", 0.3f);
				outOfBreath = true;
			}

			float lerpedDistortion = glm::lerp(_scene->MainCamera->LensDepth, 2.0f, deltaTime * 0.5f);
			_scene->MainCamera->LensDepth = lerpedDistortion;

			playerEmmiters[playerEmmiterIndex]->targetVolume = chokeVol;
		}

		glm::vec4 newCol = glm::mix(curCol, glm::vec4(0.0f, 0.4f, 0.8f, 0.6f), 2.0f * deltaTime);
		_scene->uiImages[2]->GetChildren()[0]->Get<GuiPanel>()->SetColor(newCol);
	}

	if (glfwGetKey(_window, GLFW_KEY_LEFT_CONTROL) && oxygenMeter > 0.01f)//Hold Breath
	{
		SetSpeed(1.0f);

		if (oxygenMeter > 0.01f)
		{
			if (!holdingBreath)
			{
				_scene->audioManager->Get<AudioManager>()->PlaySoundByName("HoldingBreath", 0.4f);
				holdingBreath = true;
			}
			oxygenMeter -= breathHoldDecaySpeed * deltaTime;
			playerEmmiters[playerEmmiterIndex]->targetVolume = 0.01f;
			LerpHeight(-1.0f, deltaTime, 4.0f);

		}
		else
		{
			oxygenMeter = 0.01f;
		}

		glm::vec4 newCol = glm::mix(curCol, glm::vec4(0.0f, 0.3f, 0.6f, 0.6f), 3.0f * deltaTime);
		_scene->uiImages[2]->GetChildren()[0]->Get<GuiPanel>()->SetColor(newCol);
	}
	else
	{
		if (playerPulseTimer <= 0.5f && playerState != Idle)
			LerpHeight(0.1f, deltaTime, 4.0f);
		else
			LerpHeight(0.0f, deltaTime, 4.0f);


		if (holdingBreath)
		{
			_scene->audioManager->Get<AudioManager>()->PlaySoundByName("BreathOut", 0.4f);
			holdingBreath = false;
		}
	}

	//I tested out having the oxygen level affect the sound ring colour, I don't think it's noticible enough to replace the UI.
	//soundEmmiter->colour = soundEmmiter->defaultColour * (oxygenMeter / oxygenMeterMax);
}

void SimpleCameraControl::LerpHeight(float heightOffset, float deltaTime, float speed)
{
	if (freecam)
		return;
	glm::vec3 playerPos = GetGameObject()->GetPosition();
	//if (heightOffset != 0 && glm::abs(playerPos.z - (baseHeight + heightOffset) < 0.01f))
	//	return;

	GetGameObject()->SetPostion(glm::lerp(playerPos, glm::vec3(playerPos.x, playerPos.y, baseHeight + heightOffset), speed * deltaTime));

}

void SimpleCameraControl::SwitchState(float deltaTime)
{
	//if (playerState != prevState) //When running into objects, this makes sounds go off like crazy.
	//	playerPulseTimer = 0.0001f; // This was honestly a bit unnessicary anyways.

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
	btCollisionWorld::ClosestRayResultCallback hit(ToBt(GetGameObject()->GetPosition()), ToBt(GetGameObject()->GetPosition() + (viewDir * 5.0f)));
	_scene->GetPhysicsWorld()->rayTest(ToBt(GetGameObject()->GetPosition()), ToBt(GetGameObject()->GetPosition() + (viewDir * 5.0f)), hit);

	if (!hit.hasHit())
	{
		interactionObjectPos = glm::vec3(0.0f);
		return;
	}

	glm::vec3 objectPos = ToGlm(hit.m_collisionObject->getWorldTransform().getOrigin());
	interactionObjectPos = objectPos;
}

glm::vec3 SimpleCameraControl::WhatAreYouLookingAt() {
	btCollisionWorld::ClosestRayResultCallback hit(ToBt(GetGameObject()->GetPosition()), ToBt(GetGameObject()->GetPosition() + (viewDir * 5.0f)));
	_scene->GetPhysicsWorld()->rayTest(ToBt(GetGameObject()->GetPosition()), ToBt(GetGameObject()->GetPosition() + (viewDir * 5.0f)), hit);

	if (!hit.hasHit())
	{
		return glm::vec3(0.0f);
	}

	glm::vec3 objectPos = ToGlm(hit.m_hitPointWorld);//ToGlm(hit.m_collisionObject->getWorldTransform().getOrigin());
	return objectPos;
}

void SimpleCameraControl::EmitSound(float deltaTime)
{
	if (playerPulseTimer <= 0.f)
	{
		playerPulseTimer = 1.0f;

		playerEmmiters[playerEmmiterIndex]->isDecaying = false;
		playerEmmiters[playerEmmiterIndex]->volume = 0.0f;
		playerEmmiters[playerEmmiterIndex]->MoveToPlayer();

		soundDelayTimer = soundDelayTimerMax;
		startSoundDelay = true;

		if (playerEmmiterIndex < playerEmmiterCount - 1)
			playerEmmiterIndex++;
		else
			playerEmmiterIndex = 0;
	}

	if (startSoundDelay)
	{
		soundDelayTimer -= deltaTime;
		if (soundDelayTimer <= 0.0f)
		{
			if (playerState != Idle)
				_scene->audioManager->Get<AudioManager>()->PlayFootstepSound(GetGameObject()->GetPosition() - glm::vec3(0, 0, -5.0f), playerEmmiters[playerEmmiterIndex]->targetVolume / 5.0f);
			startSoundDelay = false;
		}
	}

	prevState = playerState;
	//std::cout << "\nPulse Timer: " << playerPulseTimer;
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

void SimpleCameraControl::ShowLocked()
{
	_scene->uiImages[3]->GetChildren()[0]->Get<GuiPanel>()->SetColor(glm::vec4(1.0f));
	_scene->uiImages[3]->GetChildren()[0]->Get<GuiPanel>()->SetTexture(p_Locked);
	promptShown = true;
}

void SimpleCameraControl::ShowDropThrow()
{
	_scene->uiImages[3]->GetChildren()[0]->Get<GuiPanel>()->SetColor(glm::vec4(1.0f));
	_scene->uiImages[3]->GetChildren()[0]->Get<GuiPanel>()->SetTexture(p_DropThrow);
	PlaceUI(3, 53.33f, 30, 1, 0, 12, 1); // Interaction Prompt

	promptShown = true;
}

void SimpleCameraControl::ShowRead()
{
	_scene->uiImages[3]->GetChildren()[0]->Get<GuiPanel>()->SetColor(glm::vec4(1.0f));
	_scene->uiImages[3]->GetChildren()[0]->Get<GuiPanel>()->SetTexture(p_Read);
	promptShown = true;
}

void SimpleCameraControl::ShowActivate()
{
	_scene->uiImages[3]->GetChildren()[0]->Get<GuiPanel>()->SetColor(glm::vec4(1.0f));
	_scene->uiImages[3]->GetChildren()[0]->Get<GuiPanel>()->SetTexture(p_Activate);
	promptShown = true;
}

void SimpleCameraControl::ShowGameOver()
{
	_scene->uiImages[4]->GetChildren()[0]->Get<GuiPanel>()->SetTexture(gameoverTex);
	_scene->uiImages[4]->GetChildren()[0]->Get<GuiPanel>()->SetColor(glm::vec4(1.0f));
	promptShown = true;
}

void SimpleCameraControl::ShowLoading()
{
	_scene->uiImages[4]->GetChildren()[0]->Get<GuiPanel>()->SetTexture(loadingTex);
	_scene->uiImages[4]->GetChildren()[0]->Get<GuiPanel>()->SetColor(glm::vec4(1.0f));
	PlaceUI(4, windx / 4.0f, windy / 4.0f, 1, 0, 2, 1); // Game Over Screen
	promptShown = true;
}

void SimpleCameraControl::FadeInBlack(float deltaTime)
{
	float lerpedAlpha = glm::lerp(_scene->uiImages[4]->GetChildren()[0]->Get<GuiPanel>()->GetColor().a, 1.0f, deltaTime);
	_scene->uiImages[4]->GetChildren()[0]->Get<GuiPanel>()->SetColor(glm::vec4(1.0f, 1.0f, 1.0f, lerpedAlpha));
}

void SimpleCameraControl::FadeOutBlack(float deltaTime)
{
	if (_scene->uiImages[4]->GetChildren()[0]->Get<GuiPanel>()->GetColor().a <= 0.0001f)
	{
		_scene->uiImages[4]->GetChildren()[0]->Get<GuiPanel>()->SetColor(glm::vec4(1.0f, 1.0f, 1.0f, 0.0f));
		return;
	}
	float lerpedAlpha = glm::lerp(_scene->uiImages[4]->GetChildren()[0]->Get<GuiPanel>()->GetColor().a, 0.0f, deltaTime);
	_scene->uiImages[4]->GetChildren()[0]->Get<GuiPanel>()->SetColor(glm::vec4(1.0f, 1.0f, 1.0f, lerpedAlpha));

}

void SimpleCameraControl::ShowBlack()
{
	_scene->uiImages[4]->GetChildren()[0]->Get<GuiPanel>()->SetTexture(blackTex);
	_scene->uiImages[4]->GetChildren()[0]->Get<GuiPanel>()->SetColor(glm::vec4(1.0f));

}



void SimpleCameraControl::IdleState(float deltaTime)
{
	SetSpeed(walkSpeed);
	idleTimer -= deltaTime;
	playerPulseTimer -= deltaTime * 0.5f; //ring creation speed
	//playerPulseTimer -= deltaTime * 2.0f;

	if (idleTimer <= 0.0f)
	{
		playerEmmiters[playerEmmiterIndex]->lerpSpeed = 0.5f; //ring expansion speed

		if (inhale)
		{
			inhale = false;
			_scene->audioManager->Get<AudioManager>()->PlaySoundByName("IdleOut", 0.3f);
		}

		else
		{
			inhale = true;
			_scene->audioManager->Get<AudioManager>()->PlaySoundByName("IdleIn", 0.3f);
		}

		idleTimer = idleTimerDefault;
	}

	if (inhale)
		playerEmmiters[playerEmmiterIndex]->targetVolume = 3.5f; //ring size
	else
		playerEmmiters[playerEmmiterIndex]->targetVolume = 4.5f; //ring size

}
//This state is no longer used in the game.
void SimpleCameraControl::SneakState(float deltaTime)
{
	SetSpeed(sneakSpeed);
	playerEmmiters[playerEmmiterIndex]->targetVolume = sneakSpeed;
	playerEmmiters[playerEmmiterIndex]->lerpSpeed = 1.0f;
}

void SimpleCameraControl::WalkState(float deltaTime)
{
	SetSpeed(walkSpeed);
	playerPulseTimer -= deltaTime * 2.0f; //ring creation speed

	playerEmmiters[playerEmmiterIndex]->targetVolume = 6.5f; //ring size
	playerEmmiters[playerEmmiterIndex]->lerpSpeed = 3.0f; //ring expansion speed
}

void SimpleCameraControl::RunState(float deltaTime)
{
	SetSpeed(runSpeed);
	playerPulseTimer -= deltaTime * 2.5f; //ring creation speed

	playerEmmiters[playerEmmiterIndex]->targetVolume = 7.5f; //ring size
	playerEmmiters[playerEmmiterIndex]->lerpSpeed = 3.0f; //ring expansion speed
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
	if (!promptShown)
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
	ImGui::Text((~InputEngine::GetMouseState(GLFW_MOUSE_BUTTON_LEFT)).c_str());
	glm::dvec2 delta = InputEngine::GetMousePos() - _prevMousePos;
	ImGui::Text("%d, %d", delta.x, delta.y);
	LABEL_LEFT(ImGui::DragFloat3, "Starting Position", &startingPos.x);

}

nlohmann::json SimpleCameraControl::ToJson() const {
	return {
		{ "mouse_sensitivity", _mouseSensitivity},
		{ "move_speed", _moveSpeeds },
		{ "shift_mult", _shiftMultipler },
		{ "starting_pos", startingPos }
	};
}

SimpleCameraControl::Sptr SimpleCameraControl::FromJson(const nlohmann::json& blob) {
	SimpleCameraControl::Sptr result = std::make_shared<SimpleCameraControl>();
	result->_mouseSensitivity = JsonGet(blob, "mouse_sensitivity", result->_mouseSensitivity);
	result->_moveSpeeds = JsonGet(blob, "move_speed", result->_moveSpeeds);
	result->_shiftMultipler = JsonGet(blob, "shift_mult", 2.0f);
	result->startingPos = JsonGet(blob, "starting_pos", result->startingPos);

	return result;
}
