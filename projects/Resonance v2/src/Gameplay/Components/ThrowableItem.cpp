#include "ThrowableItem.h"
#include "Gameplay/Components/SoundEmmiter.h"
#include "Utils/ImGuiHelper.h"
#include "Utils/JsonGlmHelpers.h"
#include "Gameplay/Components/AudioManager.h"
#include "Gameplay/Components/SimpleCameraControl.h"


void ThrowableItem::Awake()
{
	scene = GetGameObject()->GetScene();
	player = scene->MainCamera->GetGameObject();

}

void ThrowableItem::Update(float deltaTime)
{
	//apply gravity force

	if (player->Get<SimpleCameraControl>()->interactionObjectPos == GetGameObject()->GetPosition())
	{
		//Show pick up prompt
		player->Get<SimpleCameraControl>()->ShowPickup();

		//if pressing e pick up
	}

}

void ThrowableItem::RenderImGui() {
	//LABEL_LEFT(ImGui::Checkbox, "muteAtZero", &muteAtZero);
	//LABEL_LEFT(ImGui::DragFloat, "distractionVolume", &distractionVolume);
	//LABEL_LEFT(ImGui::DragFloat3, "defaultColour", &defaultColour.x);

	//// Draw a textbox for our track name
	//static char nameBuff[256];
	//memcpy(nameBuff, soundName.c_str(), soundName.size());
	//nameBuff[soundName.size()] = '\0';
	//if (ImGui::InputText("", nameBuff, 256)) {
	//	soundName = nameBuff;
	//}

	//LABEL_LEFT(ImGui::DragFloat, "soundVol", &soundVol);

}

nlohmann::json ThrowableItem::ToJson() const {

	nlohmann::json result;
	//result["StartingPosition"] = startPos;
	return result;

}

ThrowableItem::Sptr ThrowableItem::FromJson(const nlohmann::json& blob) {
	ThrowableItem::Sptr result = std::make_shared<ThrowableItem>();

	//if (JsonGet(blob, "muteAtZero", glm::vec3(1)) == glm::vec3(1))
	//	result->muteAtZero = true;
	//else
	//	result->muteAtZero = false;

	//result->distractionVolume = JsonGet(blob, "distractionVolume", glm::vec3(result->distractionVolume)).x;
	//result->defaultColour = JsonGet(blob, "defaultColour", result->defaultColour);
	//result->soundName = JsonGet(blob, "soundName", result->soundName);
	//result->soundVol = JsonGet(blob, "soundVol", 1.0f);

	return result;
}

