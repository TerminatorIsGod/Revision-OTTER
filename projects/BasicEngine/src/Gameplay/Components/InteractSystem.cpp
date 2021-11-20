#include "Gameplay/Components/InteractSystem.h"
#include <GLFW/glfw3.h>
#include "Gameplay/GameObject.h"
#include "Gameplay/Scene.h"
#include "Utils/ImGuiHelper.h"
#include <Gameplay/Components/InventorySystem.h>

void InteractSystem::Awake()
{
	_window = GetGameObject()->GetScene()->Window;
}

void InteractSystem::RenderImGui() {
	LABEL_LEFT(ImGui::DragFloat, "Distance", &_distance, 1.0f);
	LABEL_LEFT(ImGui::DragFloat, "Interact Distance", &_interactDistance, 1.0f);
}

nlohmann::json InteractSystem::ToJson() const {
	return {
		{ "Distance", _distance},
		{ "InteractDistance", _interactDistance }
	};
}

InteractSystem::InteractSystem() :
	IComponent(),
	_interactDistance(0),
	_distance(0)
{ }

InteractSystem::~InteractSystem() = default;

InteractSystem::Sptr InteractSystem::FromJson(const nlohmann::json& blob) {
	InteractSystem::Sptr result = std::make_shared<InteractSystem>();
	result->_distance = blob["Distance"];
	result->_interactDistance = blob["InteractDistance"];
	return result;
}

void InteractSystem::Update(float deltaTime) {
	if (glfwGetKey(_window, GLFW_KEY_E)) {

		if (_requiresKey) {

			if (_player->Get<InventorySystem>()->getKey(_requiredKey))
				interact();

		}
		else
			interact();

		
	}
}

void InteractSystem::interact() {

	_distance = _player->GetPosition().length() - GetGameObject()->GetPosition().length();

	if (_distance < _interactDistance) {
		
		std::cout << GetGameObject()->Name << " OBJECT INTERACTED";
	}

}

