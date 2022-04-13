#include "Gameplay/Components/InteractSystem.h"
#include <GLFW/glfw3.h>
#include "Gameplay/GameObject.h"
#include "Gameplay/Scene.h"
#include "Utils/ImGuiHelper.h"
#include <Gameplay/Components/InventorySystem.h>
#include "Gameplay/Components/SimpleCameraControl.h"
#include "Gameplay/InputEngine.h"
#include "Application/Application.h"
#include "Gameplay/Components/AudioManager.h"
#include <Gameplay/Components/InteractSystem2.h>

void InteractSystem::Awake()
{
	_player = GetGameObject()->GetScene()->MainCamera->GetGameObject()->GetParent();
	_lerpS = GetGameObject()->Get<LerpSystem>();
	_slideLerp = GetGameObject()->Get<SlideLerpSystem>();
	Application& app = Application::Get();
	_window = app.GetWindow();
}

void InteractSystem::RenderImGui() {
	LABEL_LEFT(ImGui::DragFloat, "Distance", &_distance, 1.0f);
	LABEL_LEFT(ImGui::DragFloat, "Interact Distance", &_interactDistance, 1.0f);
	LABEL_LEFT(ImGui::Checkbox, "Requires Key", &_requiresKey);
	LABEL_LEFT(ImGui::Checkbox, "Is this a Key", &_iskey);
	LABEL_LEFT(ImGui::Checkbox, "Is this a Generator", &_isGenerator);
	LABEL_LEFT(ImGui::Checkbox, "Is this locked default by generator", &_isDefaultLockedByGenerator);
	LABEL_LEFT(ImGui::InputInt, "Key", &_requiredKey, 1.0f);
}

nlohmann::json InteractSystem::ToJson() const {
	return {
		{ "Distance", _distance},
		{ "InteractDistance", _interactDistance },
		{ "RequiresKey", _requiresKey},
		{ "RequiredKey", _requiredKey},
		{ "IsKey", _iskey},
		{ "IsGenerator", _isGenerator},
		{ "IsLockedByDefaultByGenerator", _isDefaultLockedByGenerator},
		{ "isLockedAfterGenIsOn", _isLockedAfterGenIsOn}
	};
}

InteractSystem::InteractSystem() :
	IComponent(),
	_interactDistance(0),
	_distance(0),
	_requiresKey(0),
	_requiredKey(0),
	_iskey(0),
	_isGenerator(0),
	_isDefaultLockedByGenerator(0),
	_isLockedAfterGenIsOn(0)
{ }

InteractSystem::~InteractSystem() = default;

InteractSystem::Sptr InteractSystem::FromJson(const nlohmann::json & blob) {
	InteractSystem::Sptr result = std::make_shared<InteractSystem>();
	result->_distance = blob["Distance"];
	result->_interactDistance = blob["InteractDistance"];
	result->_requiredKey = blob["RequiredKey"];
	result->_requiresKey = blob["RequiresKey"];
	result->_iskey = blob["IsKey"];
	result->_isGenerator = JsonGet(blob, "IsGenerator", result->_isGenerator);
	result->_isDefaultLockedByGenerator = JsonGet(blob, "IsLockedByDefaultByGenerator", result->_isDefaultLockedByGenerator);
	result->_isLockedAfterGenIsOn = JsonGet(blob, "isLockedAfterGenIsOn", result->_isLockedAfterGenIsOn);

	return result;
}

void InteractSystem::Update(float deltaTime) {

	glm::vec3 ppos = _player->GetPosition();
	glm::vec3 opos = GetGameObject()->GetPosition();

	glm::vec3 tpos = ppos - opos;
	_distance = sqrt(pow(tpos.x, 2) + pow(tpos.y, 2) + pow(tpos.z, 2));

	if (GetGameObject()->Name == "ElevatorKeycardObject") {
		if (_player->Get<SimpleCameraControl>()->interactionObjectPos == opos && !_player->Get<SimpleCameraControl>()->promptShown) {
			if (_player->Get<InventorySystem>()->getKey(_requiredKey)) {
				_player->Get<SimpleCameraControl>()->ShowActivate();
			}
			else {
				_player->Get<SimpleCameraControl>()->ShowLocked();
			}
		}
	}

	if (!(GetGameObject()->Name == "Map - Elevator Door Left") && !(GetGameObject()->Name == "Map - Elevator Door Right")) {
		if (GetGameObject()->GetScene()->isGeneratorOn) {
			if (_isLockedAfterGenIsOn && _player->Get<SimpleCameraControl>()->interactionObjectPos == opos && !_player->Get<SimpleCameraControl>()->promptShown) {
				_player->Get<SimpleCameraControl>()->ShowLocked();
				return;
			}
		}

		//Key (proximity based)
		if (_iskey && _distance <= _interactDistance && !_player->Get<SimpleCameraControl>()->promptShown) {
			_player->Get<SimpleCameraControl>()->ShowPickup();
		}

		//Animated Objects (raycast based)
		if (!_iskey && !_isGenerator && _player->Get<SimpleCameraControl>()->interactionObjectPos == opos && !_player->Get<SimpleCameraControl>()->promptShown) {

			if (_player->Get<InventorySystem>()->getKey(_requiredKey))
			{
				if (_isDefaultLockedByGenerator) {
					if (GetGameObject()->GetScene()->isGeneratorOn) {
						if (!isOpen)
							_player->Get<SimpleCameraControl>()->ShowOpen();
						else if (isOpen)
							_player->Get<SimpleCameraControl>()->ShowClose();
					}
					else {
						_player->Get<SimpleCameraControl>()->ShowLocked();
					}
				}
				if (!isOpen && !_isDefaultLockedByGenerator)
					_player->Get<SimpleCameraControl>()->ShowOpen();
				else if (isOpen)
					_player->Get<SimpleCameraControl>()->ShowClose();
			}
			else
			{
				_player->Get<SimpleCameraControl>()->ShowLocked();
			}
		}

		if (_isGenerator && _player->Get<SimpleCameraControl>()->interactionObjectPos == opos && !GetGameObject()->GetScene()->isGeneratorOn) {
			_player->Get<SimpleCameraControl>()->ShowActivate();
		}

		if (glfwGetKey(_window, GLFW_KEY_E)) {
			if (!isKeyPressed)
			{
				if (GetGameObject()->Name == "ElevatorKeycardObject") {
					if (_player->Get<SimpleCameraControl>()->interactionObjectPos == opos && _player->Get<InventorySystem>()->getKey(_requiredKey)) {
						GetGameObject()->GetScene()->isElevatorKeycard = true;
					}
				}

				
				if (_isDefaultLockedByGenerator) {
					if (GetGameObject()->GetScene()->isGeneratorOn)
						interact();
					else
						return;
				}

				if (_requiresKey) {
					if (_player->Get<InventorySystem>()->getKey(_requiredKey)) {
						interact();
					}
				}
				else {
					interact();
				}
				isKeyPressed = true;
			}
		}
		else
		{
			isKeyPressed = false;
		}
	}


}

void InteractSystem::interact() {

	_distance = (_player->GetPosition().length() - GetGameObject()->GetPosition().length());

	glm::vec3 ppos = _player->GetPosition();
	glm::vec3 opos = GetGameObject()->GetPosition();

	glm::vec3 tpos = ppos - opos;
	_distance = sqrt(pow(tpos.x, 2) + pow(tpos.y, 2) + pow(tpos.z, 2));

	//Animated Object (based on raycast)
	if (_lerpS && _player->Get<SimpleCameraControl>()->interactionObjectPos == opos) {
		_lerpS->lerpReverse = isOpen;
		_lerpS->beginLerp = true;
		if (isOpen)
		{
			isOpen = false;
			GetGameObject()->GetScene()->audioManager->Get<AudioManager>()->PlaySoundByName("DoorClose", 0.8f, GetGameObject()->GetPosition());
		}
		else
		{
			isOpen = true;
			GetGameObject()->GetScene()->audioManager->Get<AudioManager>()->PlaySoundByName("DoorOpen", 1.1f, GetGameObject()->GetPosition());
		}
	}
	else if (_slideLerp && _player->Get<SimpleCameraControl>()->interactionObjectPos == opos) {
		_slideLerp->lerpReverse = isOpen;
		if (_slideLerp->linkedDoor != "NULL") {
			GetGameObject()->GetScene()->FindObjectByName(_slideLerp->linkedDoor)->Get<InteractSystem>()->isOpen = isOpen;
			GetGameObject()->GetScene()->FindObjectByName(_slideLerp->linkedDoor)->Get<SlideLerpSystem>()->lerpReverse = isOpen;
			GetGameObject()->GetScene()->FindObjectByName(_slideLerp->linkedDoor)->Get<SlideLerpSystem>()->beginLerp = true;
		}
		_slideLerp->beginLerp = true;
		if (isOpen)
		{
			isOpen = false;
			GetGameObject()->GetScene()->audioManager->Get<AudioManager>()->PlaySoundByName("DoorClose", 0.8f, GetGameObject()->GetPosition());
		}
		else
		{
			isOpen = true;
			GetGameObject()->GetScene()->audioManager->Get<AudioManager>()->PlaySoundByName("DoorOpen", 1.1f, GetGameObject()->GetPosition());
		}
	}

	if (_isGenerator && _player->Get<SimpleCameraControl>()->interactionObjectPos == opos && !GetGameObject()->GetScene()->isGeneratorOn) {
		GetGameObject()->GetScene()->isGeneratorOn = true;

		GetGameObject()->GetScene()->FindObjectByName("ElevatorStatusScreenOff")->Get<RenderComponent>()->IsEnabled = false;
		GetGameObject()->GetScene()->FindObjectByName("ElevatorStatusScreenOn")->Get<RenderComponent>()->IsEnabled = true;

		GetGameObject()->GetScene()->FindObjectByName("Map - Elevator Door Left")->Get<SlideLerpSystem>()->lerpReverse = false;
		GetGameObject()->GetScene()->FindObjectByName("Map - Elevator Door Right")->Get<SlideLerpSystem>()->lerpReverse = false;

		GetGameObject()->GetScene()->FindObjectByName("Map - Elevator Door Left")->Get<InteractSystem2>()->isOpen = true;
		GetGameObject()->GetScene()->FindObjectByName("Map - Elevator Door Right")->Get<InteractSystem2>()->isOpen = true;

		GetGameObject()->GetScene()->FindObjectByName("Map - Elevator Door Left")->Get<SlideLerpSystem>()->beginLerp = true;
		GetGameObject()->GetScene()->FindObjectByName("Map - Elevator Door Right")->Get<SlideLerpSystem>()->beginLerp = true;



		GetGameObject()->GetScene()->FindObjectByName("Map - Glass Door 1")->Get<InteractSystem2>()->isOpen = false;
		GetGameObject()->GetScene()->FindObjectByName("Map - Glass Door 2")->Get<InteractSystem2>()->isOpen = false;
		GetGameObject()->GetScene()->FindObjectByName("Map - Glass Door 3")->Get<InteractSystem2>()->isOpen = false;
		GetGameObject()->GetScene()->FindObjectByName("Map - Glass Door 4")->Get<InteractSystem2>()->isOpen = true;
		GetGameObject()->GetScene()->FindObjectByName("Map - Glass Door 5")->Get<InteractSystem2>()->isOpen = false;
		GetGameObject()->GetScene()->FindObjectByName("Map - Glass Door 6")->Get<InteractSystem2>()->isOpen = false;

		GetGameObject()->GetScene()->FindObjectByName("Map - Glass Door 1")->Get<SlideLerpSystem>()->beginLerp = true;
		GetGameObject()->GetScene()->FindObjectByName("Map - Glass Door 2")->Get<SlideLerpSystem>()->beginLerp = true;
		GetGameObject()->GetScene()->FindObjectByName("Map - Glass Door 3")->Get<SlideLerpSystem>()->beginLerp = true;
		GetGameObject()->GetScene()->FindObjectByName("Map - Glass Door 4")->Get<SlideLerpSystem>()->beginLerp = true;
		GetGameObject()->GetScene()->FindObjectByName("Map - Glass Door 5")->Get<SlideLerpSystem>()->beginLerp = true;
		GetGameObject()->GetScene()->FindObjectByName("Map - Glass Door 6")->Get<SlideLerpSystem>()->beginLerp = true;

		GetGameObject()->GetScene()->FindObjectByName("Map - Glass Door 1")->Get<SlideLerpSystem>()->beginLerp = true;
		GetGameObject()->GetScene()->FindObjectByName("Map - Glass Door 2")->Get<SlideLerpSystem>()->beginLerp = true;
		GetGameObject()->GetScene()->FindObjectByName("Map - Glass Door 3")->Get<SlideLerpSystem>()->beginLerp = true;
		GetGameObject()->GetScene()->FindObjectByName("Map - Glass Door 4")->Get<SlideLerpSystem>()->beginLerp = true;
		GetGameObject()->GetScene()->FindObjectByName("Map - Glass Door 5")->Get<SlideLerpSystem>()->beginLerp = true;
		GetGameObject()->GetScene()->FindObjectByName("Map - Glass Door 6")->Get<SlideLerpSystem>()->beginLerp = true;



		GetGameObject()->GetScene()->FindObjectByName("Map - Glass Double Door Right 1")->Get<SlideLerpSystem>()->lerpReverse = false;
		GetGameObject()->GetScene()->FindObjectByName("Map - Glass Double Door Left 1")->Get<SlideLerpSystem>()->lerpReverse = false;
		GetGameObject()->GetScene()->FindObjectByName("Map - Glass Double Door Right 2")->Get<SlideLerpSystem>()->lerpReverse = false;
		GetGameObject()->GetScene()->FindObjectByName("Map - Glass Double Door Left 2")->Get<SlideLerpSystem>()->lerpReverse = false;
		GetGameObject()->GetScene()->FindObjectByName("Map - Glass Double Door Right 3")->Get<SlideLerpSystem>()->lerpReverse = false;
		GetGameObject()->GetScene()->FindObjectByName("Map - Glass Double Door Left 3")->Get<SlideLerpSystem>()->lerpReverse = false;
		GetGameObject()->GetScene()->FindObjectByName("Map - Glass Double Door Right 4")->Get<SlideLerpSystem>()->lerpReverse = false;
		GetGameObject()->GetScene()->FindObjectByName("Map - Glass Double Door Left 4")->Get<SlideLerpSystem>()->lerpReverse = false;

		GetGameObject()->GetScene()->FindObjectByName("Map - Glass Double Door Right 1")->Get<SlideLerpSystem>()->beginLerp = true;
		GetGameObject()->GetScene()->FindObjectByName("Map - Glass Double Door Left 1")->Get<SlideLerpSystem>()->beginLerp = true;
		GetGameObject()->GetScene()->FindObjectByName("Map - Glass Double Door Right 2")->Get<SlideLerpSystem>()->beginLerp = true;
		GetGameObject()->GetScene()->FindObjectByName("Map - Glass Double Door Left 2")->Get<SlideLerpSystem>()->beginLerp = true;
		GetGameObject()->GetScene()->FindObjectByName("Map - Glass Double Door Right 3")->Get<SlideLerpSystem>()->beginLerp = true;
		GetGameObject()->GetScene()->FindObjectByName("Map - Glass Double Door Left 3")->Get<SlideLerpSystem>()->beginLerp = true;
		GetGameObject()->GetScene()->FindObjectByName("Map - Glass Double Door Right 4")->Get<SlideLerpSystem>()->beginLerp = true;
		GetGameObject()->GetScene()->FindObjectByName("Map - Glass Double Door Left 4")->Get<SlideLerpSystem>()->beginLerp = true;

		GetGameObject()->GetScene()->FindObjectByName("Map - Glass Double Door Right 1")->Get<InteractSystem2>()->isOpen = false;
		GetGameObject()->GetScene()->FindObjectByName("Map - Glass Double Door Left 1")->Get<InteractSystem2>()->isOpen = false;
		GetGameObject()->GetScene()->FindObjectByName("Map - Glass Double Door Right 2")->Get<InteractSystem2>()->isOpen = false;
		GetGameObject()->GetScene()->FindObjectByName("Map - Glass Double Door Left 2")->Get<InteractSystem2>()->isOpen = false;
		GetGameObject()->GetScene()->FindObjectByName("Map - Glass Double Door Right 3")->Get<InteractSystem2>()->isOpen = false;
		GetGameObject()->GetScene()->FindObjectByName("Map - Glass Double Door Left 3")->Get<InteractSystem2>()->isOpen = false;
		GetGameObject()->GetScene()->FindObjectByName("Map - Glass Double Door Right 4")->Get<InteractSystem2>()->isOpen = false;
		GetGameObject()->GetScene()->FindObjectByName("Map - Glass Double Door Left 4")->Get<InteractSystem2>()->isOpen = false;

		GetGameObject()->GetScene()->audioManager->Get<AudioManager>()->PlaySoundByName("Generator", 1.0f, GetGameObject()->GetPosition());
		GetGameObject()->GetScene()->audioManager->Get<AudioManager>()->PlaySoundByName("Message", 1.0f, GetGameObject()->GetPosition() + glm::vec3(0.0f, 0.0f, 3.0f));
	}

	//Key (based on distance)
	if (_iskey && _distance <= _interactDistance) {
		_player->Get<InventorySystem>()->setKey(_requiredKey, true);
		GetGameObject()->GetScene()->audioManager->Get<AudioManager>()->PlaySoundByName("KeyPickup", 0.5f);
		GetGameObject()->SetPostion(glm::vec3(0, 0, -100000));
		isOpen = !isOpen;
	}

}

