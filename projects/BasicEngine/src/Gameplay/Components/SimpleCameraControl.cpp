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

SimpleCameraControl::SimpleCameraControl() :
	IComponent(),
	_mouseSensitivity({ 0.5f, 0.3f }),
	_moveSpeeds(glm::vec3(1600.0f)),
	_shiftMultipler(2.0f),
	_currentRot(glm::vec2(0.0f)),
	_isMousePressed(false)
{ }

SimpleCameraControl::~SimpleCameraControl() = default;

void SimpleCameraControl::Awake() {
	_scene = GetGameObject()->GetScene();
	_window = _scene->Window;

	soundEmmiter = GetComponent<SoundEmmiter>();
	soundEmmiter->isDecaying = false;
	soundEmmiter->soundLightOffset = glm::vec3(0, 0, -3.0f);
	glfwSetInputMode(_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}

void SimpleCameraControl::Update(float deltaTime)
{

	Movement(deltaTime);
	SwitchState(deltaTime);
	OxygenSystem(deltaTime);

	if (glfwGetKey(_window, GLFW_KEY_E))
	{
		if (!isEPressed)
		{
			isEPressed = true;
			Interact(deltaTime);
		}
	}
	else
		isEPressed = false;
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
		std::cout << "Chaning mouse tyhing\n";

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
		else if (glfwGetKey(_window, GLFW_KEY_LEFT_CONTROL))
			playerState = Sneak;
		else
			playerState = Walk;

		float velocityMagnitude = glm::sqrt((_body->GetLinearVelocity().x * _body->GetLinearVelocity().x) + (_body->GetLinearVelocity().y * _body->GetLinearVelocity().y));
		if (velocityMagnitude < 0.5f)
		{
			playerState = Idle;
		}
		else
		{
			soundEmmiter->lerpSpeed = soundEmmiter->attackSpeed;
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

		input *= deltaTime;

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

	}

}

void SimpleCameraControl::OxygenSystem(float deltaTime)
{
	//std::cout << "\nO2: " << oxygenMeter;
	//Fill Oxygen
	if (glfwGetKey(_window, GLFW_KEY_F) && !glfwGetKey(_window, GLFW_KEY_C))
	{
		if (oxygenMeter < oxygenMeterMax)
		{
			oxygenMeter += oxygenReplenishSpeed * deltaTime;
			soundEmmiter->targetVolume = replenishVol;
		}
		else
		{
			oxygenMeter = oxygenMeterMax;
		}
	}
	//Oxygen Decay
	else
	{
		if (oxygenMeter > 0)
		{
			oxygenMeter -= oxygenDecaySpeed * deltaTime;
		}
		else
		{
			oxygenMeter = 0;
			soundEmmiter->targetVolume = chokeVol;
		}
	}

	if (glfwGetKey(_window, GLFW_KEY_C))//Hold Breath
	{
		SetSpeed(1.0f);

		if (oxygenMeter > 0)
		{
			oxygenMeter -= breathHoldDecaySpeed * deltaTime;
			soundEmmiter->targetVolume = 0.0f;
		}
		else
		{
			oxygenMeter = 0.0f;
		}
	}

	//I tested out having the oxygen level affect the sound ring colour, I don't think it's noticible enough to replace the UI.
	//soundEmmiter->colour = soundEmmiter->defaultColour * (oxygenMeter / oxygenMeterMax);
}


void SimpleCameraControl::SwitchState(float deltaTime)
{
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
	glm::vec3 viewDir = currentRot * glm::vec4(0.0f, 0.0f, -1.0f, 1.0f);
	btCollisionWorld::ClosestRayResultCallback hit(ToBt(GetGameObject()->GetPosition()), ToBt(GetGameObject()->GetPosition() + (viewDir * 5.0f)));
	_scene->GetPhysicsWorld()->rayTest(ToBt(GetGameObject()->GetPosition()), ToBt(GetGameObject()->GetPosition() + (viewDir * 5.0f)), hit);

	if (!hit.hasHit())
		return;


	glm::vec3 objectPos = ToGlm(hit.m_collisionObject->getWorldTransform().getOrigin());

	if (objectPos == glm::vec3(0))
		return;

	for (int i = 0; i < _scene->soundEmmiters.size(); i++)
	{
		if (objectPos == _scene->soundEmmiters[i]->GetPosition())
		{
			_scene->soundEmmiters[i]->Get<SoundEmmiter>()->targetVolume = _scene->soundEmmiters[i]->Get<SoundEmmiter>()->distractionVolume;
			_scene->soundEmmiters[i]->Get<SoundEmmiter>()->isDecaying = false;
			_scene->soundEmmiters[i]->Get<SoundEmmiter>()->lerpSpeed = 4.0f;
		}
		//std::cout << "\nObject Pos: " << _scene->soundEmmiters[i]->GetPosition().y;
	}

	for (int i = 0; i < _scene->ladders.size(); i++)
	{
		if (objectPos == _scene->ladders[i]->GetPosition())
			GetGameObject()->SetPostion(_scene->ladders[i]->Get<Ladder>()->teleportPos);
	}
}

void SimpleCameraControl::IdleState(float deltaTime)
{
	SetSpeed(walkSpeed);

	idleTimer -= deltaTime;
	if (idleTimer <= 0.0f)
	{
		soundEmmiter->lerpSpeed = 1.0f;

		if (inhale)
			inhale = false;
		else
			inhale = true;

		idleTimer = idleTimerDefault;
	}

	if (inhale)
		soundEmmiter->targetVolume = 4.5f;
	else
		soundEmmiter->targetVolume = 5.5f;
}

void SimpleCameraControl::SneakState(float deltaTime)
{
	SetSpeed(sneakSpeed);
	soundEmmiter->targetVolume = sneakSpeed;
}

void SimpleCameraControl::WalkState(float deltaTime)
{
	SetSpeed(walkSpeed);
	soundEmmiter->targetVolume = walkSpeed;
}

void SimpleCameraControl::RunState(float deltaTime)
{
	SetSpeed(runSpeed);
	soundEmmiter->targetVolume = runSpeed;
}

void SimpleCameraControl::SetSpeed(float newSpeed)
{
	_moveSpeeds = glm::vec3(newSpeed * 300.0f);
}



void SimpleCameraControl::RenderImGui()
{
	LABEL_LEFT(ImGui::DragFloat2, "Mouse Sensitivity", &_mouseSensitivity.x, 0.01f);
	LABEL_LEFT(ImGui::DragFloat3, "Move Speed       ", &_moveSpeeds.x, 0.01f, 0.01f);
	LABEL_LEFT(ImGui::DragFloat, "Shift Multiplier ", &_shiftMultipler, 0.01f, 1.0f);
}

nlohmann::json SimpleCameraControl::ToJson() const {
	return {
		{ "mouse_sensitivity", GlmToJson(_mouseSensitivity) },
		{ "move_speed", GlmToJson(_moveSpeeds) },
		{ "shift_mult", _shiftMultipler }
	};
}

SimpleCameraControl::Sptr SimpleCameraControl::FromJson(const nlohmann::json & blob) {
	SimpleCameraControl::Sptr result = std::make_shared<SimpleCameraControl>();
	result->_mouseSensitivity = ParseJsonVec2(blob["mouse_sensitivity"]);
	result->_moveSpeeds = ParseJsonVec3(blob["move_speed"]);
	result->_shiftMultipler = JsonGet(blob, "shift_mult", 2.0f);
	return result;
}
