#include "Gameplay/Components/SoundEmmiter.h"
#include "Utils/ImGuiHelper.h"
#include "Utils/JsonGlmHelpers.h"


void SoundEmmiter::Awake()
{
	lerpSpeed = attackSpeed;

	scene = GetGameObject()->GetScene();

	scene->Lights.push_back(Light());
	soundLight = scene->Lights.size() - 1;
	scene->SetupShaderAndLights();

	scene->soundEmmiters.push_back(GetGameObject());

	colour = defaultColour;
	scene->Lights[soundLight].Color = colour;

}

void SoundEmmiter::Update(float deltaTime)
{
	if (isDecaying)
	{
		if (muteAtZero && volume < 0.1f)
			volume = -1.0f;
		else
			Decay(deltaTime);
	}
	else
	{
		Attack(deltaTime);
	}

	scene->Lights[soundLight].Range = -volume * soundLightMultiplier;
	scene->Lights[soundLight].Position = GetGameObject()->GetPosition();
	//std::cout << "\nLight:" << " | " << soundLight->Position.y;

	//soundLight->Color = colour;

}

void SoundEmmiter::RenderImGui() {
	LABEL_LEFT(ImGui::Checkbox, "muteAtZero", &muteAtZero);
	LABEL_LEFT(ImGui::DragFloat, "distractionVolume", &distractionVolume);
	LABEL_LEFT(ImGui::DragFloat3, "defaultColour", &defaultColour.x);
}

nlohmann::json SoundEmmiter::ToJson() const {
	return {
		{ "muteAtZero", GlmToJson(glm::vec3(muteAtZero)) },
		{ "distractionVolume", GlmToJson(glm::vec3(distractionVolume)) },
		{ "defaultColour", GlmToJson(defaultColour) }
	};
}

SoundEmmiter::Sptr SoundEmmiter::FromJson(const nlohmann::json& data) {
	SoundEmmiter::Sptr result = std::make_shared<SoundEmmiter>();

	if (ParseJsonVec3(data["muteAtZero"]) == glm::vec3(1))
		result->muteAtZero = true;
	else
		result->muteAtZero = false;

	result->distractionVolume = ParseJsonVec3(data["distractionVolume"]).x;
	result->defaultColour = ParseJsonVec3(data["defaultColour"]);

	return result;
}


void SoundEmmiter::Decay(float deltaTime)
{
	volume = glm::mix(volume, 0.0f, decaySpeed * deltaTime);
}

void SoundEmmiter::Attack(float deltaTime)
{
	volume = glm::mix(volume, targetVolume, lerpSpeed * deltaTime);

	if (muteAtZero && targetVolume - volume < 1.0f)
		isDecaying = true;
}
