#include "Gameplay/Components/Ladder.h"
#include "Utils/ImGuiHelper.h"
#include "Utils/JsonGlmHelpers.h"

void Ladder::Awake()
{
	GetGameObject()->GetScene()->ladders.push_back(GetGameObject());
}

void Ladder::RenderImGui() {
	LABEL_LEFT(ImGui::DragFloat3, "Teleport Position", &teleportPos.x);
}

nlohmann::json Ladder::ToJson() const {
	return {
		{ "Teleport Position", teleportPos }
	};
}

Ladder::Sptr Ladder::FromJson(const nlohmann::json& blob) {
	Ladder::Sptr result = std::make_shared<Ladder>();
	result->teleportPos = JsonGet(blob, "Teleport Position", result->teleportPos);
	return result;
}
