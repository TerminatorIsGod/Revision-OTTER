#include "Gameplay/Components/InventorySystem.h"
#include <GLFW/glfw3.h>
#include "Gameplay/GameObject.h"
#include "Gameplay/Scene.h"
#include "Utils/ImGuiHelper.h"

void InventorySystem::Awake()
{

}

void InventorySystem::RenderImGui() {
	LABEL_LEFT(ImGui::DragInt, "Keys", &_keys, 1.0f);
}

nlohmann::json InventorySystem::ToJson() const {
	return {
		{ "keys", _keys }
	};
}

InventorySystem::InventorySystem() :
	IComponent(),
	_keys(0)
{ }

InventorySystem::~InventorySystem() = default;

InventorySystem::Sptr InventorySystem::FromJson(const nlohmann::json& blob) {
	InventorySystem::Sptr result = std::make_shared<InventorySystem>();
	result->_keys = blob["keys"];
	return result;
}

void InventorySystem::Update(float deltaTime) {

}

int InventorySystem::getKeysAmount() {
	return _keys;
}

void InventorySystem::setKeysAmount(int keysAmount) {
	_keys = keysAmount;
}

void InventorySystem::addKey() {
	_keys++;
}

bool InventorySystem::useKey() {
	if (_keys > 0) {
		_keys--;
		return true;
	}

	return false;	
}

