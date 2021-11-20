#include "Gameplay/Components/SoundEmmiter.h"
#include "Utils/ImGuiHelper.h"
#include "Utils/JsonGlmHelpers.h"


void SoundEmmiter::Awake()
{
	lerpSpeed = attackSpeed;

	scene = GetGameObject()->GetScene();

	scene->Lights.push_back(Light());
	soundLight = &scene->Lights[scene->Lights.size() - 1];
	scene->soundEmmiters.push_back(GetGameObject());

	colour = defaultColour;
	soundLight->Color = colour;

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

	soundLight->Range = -volume * 16.0f;
	soundLight->Position = GetGameObject()->GetPosition();
	//std::cout << "\nLight:" << " | " << soundLight->Position.y;

	//soundLight->Color = colour;
	scene->SetupShaderAndLights();

}

void SoundEmmiter::RenderImGui() {
	LABEL_LEFT(ImGui::DragFloat3, "Speed", &speed.x);
}

nlohmann::json SoundEmmiter::ToJson() const {
	return {
		{ "speed", GlmToJson(speed) }
		//Eventually make it so it saves the nodes's nbor list. You could do this by saving the index of the node's nbors in the navNodes list.
	};
}

SoundEmmiter::Sptr SoundEmmiter::FromJson(const nlohmann::json& data) {
	SoundEmmiter::Sptr result = std::make_shared<SoundEmmiter>();
	result->speed = ParseJsonVec3(data["speed"]);
	return result;
}


void SoundEmmiter::Decay(float deltaTime)
{
	volume = glm::mix(volume, 0.0f, decaySpeed * deltaTime);
}

void SoundEmmiter::Attack(float deltaTime)
{
	volume = glm::mix(volume, targetVolume, lerpSpeed * deltaTime);
}
