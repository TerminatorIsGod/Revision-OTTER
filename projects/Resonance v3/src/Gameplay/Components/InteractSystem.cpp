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
	LABEL_LEFT(ImGui::InputInt, "Key", &_requiredKey, 1.0f);
}

nlohmann::json InteractSystem::ToJson() const {
	return {
		{ "Distance", _distance},
		{ "InteractDistance", _interactDistance },
		{ "RequiresKey", _requiresKey},
		{ "RequiredKey", _requiredKey},
		{ "IsKey", _iskey},
		{ "IsGenerator", _isGenerator}
	};
}

InteractSystem::InteractSystem() :
	IComponent(),
	_interactDistance(0),
	_distance(0),
	_requiresKey(0),
	_requiredKey(0),
	_iskey(0),
	_isGenerator(0)
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

	return result;
}

void InteractSystem::Update(float deltaTime) {

	glm::vec3 ppos = _player->GetPosition();
	glm::vec3 opos = GetGameObject()->GetPosition();

	glm::vec3 tpos = ppos - opos;
	_distance = sqrt(pow(tpos.x, 2) + pow(tpos.y, 2) + pow(tpos.z, 2));

	//Key (proximity based)
	if (_iskey && _distance <= _interactDistance && !_player->Get<SimpleCameraControl>()->promptShown) {
		_player->Get<SimpleCameraControl>()->ShowPickup();
	}

	//Animated Objects (raycast based)
	if (!_iskey && !_isGenerator && _player->Get<SimpleCameraControl>()->interactionObjectPos == opos && !_player->Get<SimpleCameraControl>()->promptShown) {

		if (_player->Get<InventorySystem>()->getKey(_requiredKey))
		{
			if (!isOpen)
				_player->Get<SimpleCameraControl>()->ShowOpen();
			else if (isOpen)
				_player->Get<SimpleCameraControl>()->ShowClose();
		}
		else
		{
			_player->Get<SimpleCameraControl>()->ShowLocked();
		}
	}

	if (_isGenerator && _player->Get<SimpleCameraControl>()->interactionObjectPos == opos && !_player->Get<SimpleCameraControl>()->promptShown) {
		_player->Get<SimpleCameraControl>()->ShowOpen();
	}

	if (glfwGetKey(_window, GLFW_KEY_E)) {
		if (!isKeyPressed)
		{
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

	if (_isGenerator && _distance <= _interactDistance) {
		GetGameObject()->GetScene()->isGeneratorOn = true;
		LOG_INFO("Generator is now on!");
	}

	//Key (based on distance)
	if (_iskey && _distance <= _interactDistance) {
		_player->Get<InventorySystem>()->setKey(_requiredKey, true);
		GetGameObject()->GetScene()->audioManager->Get<AudioManager>()->PlaySoundByName("KeyPickup", 0.5f);
		GetGameObject()->SetPostion(glm::vec3(0, 0, -100000));
		isOpen = !isOpen;
	}

}

