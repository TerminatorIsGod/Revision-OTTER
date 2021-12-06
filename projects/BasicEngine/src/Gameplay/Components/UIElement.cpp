#include "Gameplay/Components/UIElement.h"
#include "Utils/ImGuiHelper.h"
#include "Utils/JsonGlmHelpers.h"

void UIElement::Awake() {
	GameObject::Sptr myself(GetGameObject());
	GetGameObject()->GetScene()->uiImages.push_back(myself);
}

void UIElement::RenderImGui() {
	LABEL_LEFT(ImGui::DragFloat3, "Position Offset", &posOffset.x);
	LABEL_LEFT(ImGui::DragFloat3, "Rotation Offset", &rotOffset.x);

}

nlohmann::json UIElement::ToJson() const {
	return {
		{ "posOffset", GlmToJson(posOffset) },
		{ "rotOffset", GlmToJson(rotOffset) }

		//Eventually make it so it saves the nodes's nbor list. You could do this by saving the index of the node's nbors in the navNodes list.
	};
}

UIElement::Sptr UIElement::FromJson(const nlohmann::json& data) {
	UIElement::Sptr result = std::make_shared<UIElement>();
	result->posOffset = ParseJsonVec3(data["posOffset"]);
	result->rotOffset = ParseJsonVec3(data["rotOffset"]);

	return result;
}
