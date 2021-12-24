#include "Gameplay/Components/UIElement.h"
#include "Utils/ImGuiHelper.h"
#include "Utils/JsonGlmHelpers.h"

void UIElement::Awake() {
	GameObject::Sptr myself(GetGameObject());
	GetGameObject()->GetScene()->uiImages.push_back(myself);
}

void UIElement::RenderImGui() {
	LABEL_LEFT(ImGui::DragFloat3, "Position Offset", &posOffset.x);
}

nlohmann::json UIElement::ToJson() const {
	return {
		{ "posOffset", GlmToJson(posOffset) },
		{ "uiType", GlmToJson(glm::vec3(uiType)) }
	};
}

UIElement::Sptr UIElement::FromJson(const nlohmann::json& data) {
	UIElement::Sptr result = std::make_shared<UIElement>();
	result->posOffset = ParseJsonVec3(data["posOffset"]);
	result->uiType = (Type)ParseJsonVec3(data["uiType"]).x;
	return result;
}
