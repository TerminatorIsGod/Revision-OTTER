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
		{ "key1", key1 },
		{ "key2", key2 },
		{ "key3", key3 }
	};
}

InventorySystem::InventorySystem() :
	IComponent(),
	key1(0),
	key2(0),
	key3(0)
{ }

InventorySystem::~InventorySystem() = default;

InventorySystem::Sptr InventorySystem::FromJson(const nlohmann::json& blob) {
	InventorySystem::Sptr result = std::make_shared<InventorySystem>();
	result->key1 = blob["key1"];
	result->key2 = blob["key2"];
	result->key3 = blob["key3"];
	return result;
}

void InventorySystem::Update(float deltaTime) {

}

void InventorySystem::setKey(int key, bool value) {
	_keys[key] = value;

	key1 = _keys[0];
	key2 = _keys[1];
	key3 = _keys[2];
}

bool InventorySystem::getKey(int key) {
	return _keys[key];
}

