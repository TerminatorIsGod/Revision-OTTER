
#include "Gameplay/Components/NavNode.h"
#include "Utils/ImGuiHelper.h"
#include "Utils/JsonGlmHelpers.h"

void NavNode::Awake() {

}

void NavNode::Reset() {
	NodeType = type::blank;
	hCost = 0;
	gCost = 0;
	fCost = 0;
	parent = NULL;
}

void NavNode::ClearCosts() {
	hCost = 0;
	gCost = 0;
	fCost = 0;
	parent = NULL;
}

void NavNode::RenderImGui() {
	LABEL_LEFT(ImGui::DragFloat3, "Speed", &speed.x);
}

nlohmann::json NavNode::ToJson() const {
	return {
		{ "speed", GlmToJson(speed) }
		//Eventually make it so it saves the nodes's nbor list. You could do this by saving the index of the node's nbors in the navNodes list.
	};
}

NavNode::Sptr NavNode::FromJson(const nlohmann::json& data) {
	NavNode::Sptr result = std::make_shared<NavNode>();
	result->speed = ParseJsonVec3(data["speed"]);
	return result;
}
