#include "Gameplay/Components/SoundEmmiter.h"
#include "Utils/ImGuiHelper.h"
#include "Utils/JsonGlmHelpers.h"


void SoundEmmiter::Awake()
{
	lerpSpeed = attackSpeed;

	scene = GetGameObject()->GetScene();

	scene->Lights.push_back(Light());
	soundLight = scene->Lights.size() - 1;
	scene->Lights[soundLight].isGenerated = true;
	scene->soundEmmiters.push_back(GetGameObject());


	colour = defaultColour;
	scene->Lights[soundLight].Color = colour;

}

void SoundEmmiter::Update(float deltaTime)
{
	//if (isDecaying)
	//{
	//	if (muteAtZero && volume < 0.1f)
	//		volume = -1.0f;
	//	else
	//		Decay(deltaTime);
	//}
	//else
	//{
	//	Attack(deltaTime);
	//}

	if (!isDecaying)
		Attack(deltaTime);

	scene->Lights[soundLight].Range = volume * volume * -1.20f;
	scene->Lights[soundLight].Position = GetGameObject()->GetPosition();
}

void SoundEmmiter::RenderImGui() {
	LABEL_LEFT(ImGui::Checkbox, "muteAtZero", &muteAtZero);
	LABEL_LEFT(ImGui::DragFloat, "distractionVolume", &distractionVolume);
	LABEL_LEFT(ImGui::DragFloat3, "defaultColour", &defaultColour.x);
}

nlohmann::json SoundEmmiter::ToJson() const {
	return {
		{ "muteAtZero", glm::vec3(muteAtZero) },
		{ "distractionVolume", glm::vec3(distractionVolume) },
		{ "defaultColour", defaultColour }
	};
}

SoundEmmiter::Sptr SoundEmmiter::FromJson(const nlohmann::json& blob) {
	SoundEmmiter::Sptr result = std::make_shared<SoundEmmiter>();

	if (JsonGet(blob, "muteAtZero", glm::vec3(1)) == glm::vec3(1))
		result->muteAtZero = true;
	else
		result->muteAtZero = false;

	result->distractionVolume = JsonGet(blob, "distractionVolume", glm::vec3(result->distractionVolume)).x;
	result->defaultColour = JsonGet(blob, "defaultColour", result->defaultColour);

	return result;
}


void SoundEmmiter::Decay(float deltaTime)
{
	volume = glm::mix(volume, 0.0f, decaySpeed * deltaTime);
}

void SoundEmmiter::Attack(float deltaTime)
{
	volume = glm::mix(volume, targetVolume, lerpSpeed * deltaTime);

	if (!muteAtZero)
		return;

	scene->Lights[soundLight].Color = glm::vec3(defaultColour * (1.0f - (volume / targetVolume)));

	if (targetVolume - volume < 0.001f)
	{
		isDecaying = true;
		volume = -1.0;
	}
}
