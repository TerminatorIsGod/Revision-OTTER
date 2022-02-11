#include "Gameplay/Components/InteractSystem.h"
#include <GLFW/glfw3.h>
#include "Gameplay/GameObject.h"
#include "Gameplay/Scene.h"
#include "Utils/ImGuiHelper.h"
#include <Gameplay/Components/InventorySystem.h>
#include "Gameplay/Components/SimpleCameraControl.h"
#include "Gameplay/InputEngine.h"
#include "Application/Application.h"

void InteractSystem::Awake()
{
	_player = GetGameObject()->GetScene()->FindObjectByName("Main Camera");
	_lerpS = GetGameObject()->Get<LerpSystem>();
	Application& app = Application::Get();
	_window = app.GetWindow();
}

void InteractSystem::RenderImGui() {
	LABEL_LEFT(ImGui::DragFloat, "Distance", &_distance, 1.0f);
	LABEL_LEFT(ImGui::DragFloat, "Interact Distance", &_interactDistance, 1.0f);
	LABEL_LEFT(ImGui::Checkbox, "Requires Key", &_requiresKey);
	LABEL_LEFT(ImGui::Checkbox, "Is this a Key", &_iskey);
	LABEL_LEFT(ImGui::InputInt, "Key", &_requiredKey, 1.0f);
}

nlohmann::json InteractSystem::ToJson() const {
	return {
		{ "Distance", _distance},
		{ "InteractDistance", _interactDistance },
		{ "RequiresKey", _requiresKey},
		{ "RequiredKey", _requiredKey},
		{ "IsKey", _iskey}
	};
}

InteractSystem::InteractSystem() :
	IComponent(),
	_interactDistance(0),
	_distance(0),
	_requiresKey(0),
	_requiredKey(0),
	_iskey(0)
{ }

InteractSystem::~InteractSystem() = default;

InteractSystem::Sptr InteractSystem::FromJson(const nlohmann::json & blob) {
	InteractSystem::Sptr result = std::make_shared<InteractSystem>();
	result->_distance = blob["Distance"];
	result->_interactDistance = blob["InteractDistance"];
	result->_requiredKey = blob["RequiredKey"];
	result->_requiresKey = blob["RequiresKey"];
	result->_iskey = blob["IsKey"];
	return result;
}

void InteractSystem::Update(float deltaTime) {

	glm::vec3 ppos = _player->GetPosition();
	glm::vec3 opos = GetGameObject()->GetPosition();

	glm::vec3 tpos = ppos - opos;
	_distance = sqrt(pow(tpos.x, 2) + pow(tpos.y, 2) + pow(tpos.z, 2));

	if (_distance <= _interactDistance && !_player->Get<SimpleCameraControl>()->promptShown) {
		if (_iskey)
			_player->Get<SimpleCameraControl>()->ShowPickup();
		else if (_player->Get<InventorySystem>()->getKey(_requiredKey) && !isOpen)
			_player->Get<SimpleCameraControl>()->ShowOpen();
		else if (_player->Get<InventorySystem>()->getKey(_requiredKey) && isOpen)
			_player->Get<SimpleCameraControl>()->ShowClose();
	}

	if (glfwGetKey(_window, GLFW_KEY_E)) {
		if (!isKeyPressed)
		{
			if (_requiresKey) {

				if (_player->Get<InventorySystem>()->getKey(_requiredKey)) {
					interact();
					isOpen = !isOpen;
				}

			}
			else {
				interact();
				isOpen = !isOpen;
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

	//Gameplay::Physics::RigidBody::Sptr _body = GetGameObject()->Get<Gameplay::Physics::RigidBody>();

	//if (_body != nullptr) {
	//	_body->IsEnabled = !isOpen;
	//}


	if (_distance <= _interactDistance) {

		if (_lerpS) {
			_lerpS->lerpReverse = isOpen;
			_lerpS->beginLerp = true;
		}

		if (_iskey) {
			_player->Get<InventorySystem>()->setKey(_requiredKey, true);
			GetGameObject()->SetPostion(glm::vec3(0, 0, -100000));
		}

	}

}

