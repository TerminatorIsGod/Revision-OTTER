#include "Gameplay/Components/UIElement.h"
#include "Utils/ImGuiHelper.h"
#include "Utils/JsonGlmHelpers.h"

void UIElement::Awake() {
	GetGameObject()->GetScene()->uiImages.push_back(GetGameObject());
}

void UIElement::RenderImGui() {
	LABEL_LEFT(ImGui::DragFloat3, "Position Offset", &posOffset.x);
}

nlohmann::json UIElement::ToJson() const {
	return {
		{ "posOffset", posOffset },
		{ "uiType", glm::vec3(uiType) }
	};
}

UIElement::Sptr UIElement::FromJson(const nlohmann::json& blob) {
	UIElement::Sptr result = std::make_shared<UIElement>();
	result->posOffset = JsonGet(blob, "posOffset", result->posOffset);
	result->uiType = (Type)JsonGet(blob, "uiType", glm::vec3(result->uiType)).x;
	return result;
}
