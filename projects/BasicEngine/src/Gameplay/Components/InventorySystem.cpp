#include "Gameplay/Components/InventorySystem.h"
#include <GLFW/glfw3.h>
#include "Gameplay/GameObject.h"
#include "Gameplay/Scene.h"
#include "Utils/ImGuiHelper.h"

void InventorySystem::Awake()
{

}

void InventorySystem::RenderImGui() {
	//for (int i = 0; i < _keys.size(); i++) {
	//	LABEL_LEFT(ImGui::Checkbox, "Key " + i, &_keys[i]);
	//}

	LABEL_LEFT(ImGui::Checkbox, "Key 1", &key1);
	LABEL_LEFT(ImGui::Checkbox, "Key 2", &key2);
	LABEL_LEFT(ImGui::Checkbox, "Key 3", &key3);
}

nlohmann::json InventorySystem::ToJson() const {
	return {
	//	{ "keys", _keys }
	};
}

InventorySystem::InventorySystem() :
	IComponent()//,
	//_keys(0)
{ }

InventorySystem::~InventorySystem() = default;

InventorySystem::Sptr InventorySystem::FromJson(const nlohmann::json& blob) {
	InventorySystem::Sptr result = std::make_shared<InventorySystem>();
	//result->_keys = blob["keys"];
	return result;
}

void InventorySystem::Update(float deltaTime) {

}

void InventorySystem::setKey(int key, bool value) {
	_keys[key] = value;
}

bool InventorySystem::getKey(int key) {
	return _keys[key];
}

