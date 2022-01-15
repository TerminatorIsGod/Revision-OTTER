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
		{ "Teleport Position", GlmToJson(teleportPos) }
	};
}

Ladder::Sptr Ladder::FromJson(const nlohmann::json& data) {
	Ladder::Sptr result = std::make_shared<Ladder>();
	result->teleportPos = ParseJsonVec3(data["Teleport Position"]);
	return result;
}
