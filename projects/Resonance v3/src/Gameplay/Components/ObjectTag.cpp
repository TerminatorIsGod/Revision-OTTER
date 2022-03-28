#include "Gameplay/Components/ObjectTag.h"
#include "Utils/ImGuiHelper.h"
#include "Utils/JsonGlmHelpers.h"

void ObjectTag::RenderImGui() {
	LABEL_LEFT(ImGui::DragFloat3, "Speed", &speed.x);
}

nlohmann::json ObjectTag::ToJson() const {
	return {
		{ "speed", speed}
		//Eventually make it so it saves the nodes's nbor list. You could do this by saving the index of the node's nbors in the navNodes list.
	};
}

ObjectTag::Sptr ObjectTag::FromJson(const nlohmann::json& blob) {
	ObjectTag::Sptr result = std::make_shared<ObjectTag>();
	result->speed = JsonGet(blob, "speed", result->speed);
	return result;
}
