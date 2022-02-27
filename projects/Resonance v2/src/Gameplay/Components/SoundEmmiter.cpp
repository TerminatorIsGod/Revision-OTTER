#include "Gameplay/Components/SoundEmmiter.h"
#include "Utils/ImGuiHelper.h"
#include "Utils/JsonGlmHelpers.h"
#include "Gameplay/Components/AudioManager.h"

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
	if (!isPlayerLight)
		scene->Lights[soundLight].Position = GetGameObject()->GetPosition();
}

void SoundEmmiter::RenderImGui() {
	LABEL_LEFT(ImGui::Checkbox, "muteAtZero", &muteAtZero);
	LABEL_LEFT(ImGui::DragFloat, "distractionVolume", &distractionVolume);
	LABEL_LEFT(ImGui::DragFloat3, "defaultColour", &defaultColour.x);

	// Draw a textbox for our track name
	static char nameBuff[256];
	memcpy(nameBuff, soundName.c_str(), soundName.size());
	nameBuff[soundName.size()] = '\0';
	if (ImGui::InputText("", nameBuff, 256)) {
		soundName = nameBuff;
	}

	LABEL_LEFT(ImGui::DragFloat, "soundVol", &soundVol);

}

nlohmann::json SoundEmmiter::ToJson() const {
	return {
		{ "muteAtZero", glm::vec3(muteAtZero) },
		{ "distractionVolume", glm::vec3(distractionVolume) },
		{ "defaultColour", defaultColour },
		{ "soundName", soundName },
		{ "soundVol", soundVol }

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
	result->soundName = JsonGet(blob, "soundName", result->soundName);
	result->soundVol = JsonGet(blob, "soundVol", 1.0f);

	return result;
}


void SoundEmmiter::Decay(float deltaTime)
{
	volume = glm::mix(volume, 0.0f, decaySpeed * deltaTime);
}

void SoundEmmiter::Attack(float deltaTime)
{
	if (!soundPlayed && !isPlayerLight)
	{
		scene->audioManager->Get<AudioManager>()->PlaySoundByName(soundName, soundVol, GetGameObject()->GetPosition());
		soundPlayed = true;
	}

	if (linearLerp)
	{
		volume = glm::mix(volume, targetVolume, t);
		t += deltaTime * (lerpSpeed / 10);
	}
	else
	{
		volume = glm::mix(volume, targetVolume, lerpSpeed * deltaTime);
	}

	if (!muteAtZero)
		return;

	scene->Lights[soundLight].Color = glm::vec3(defaultColour * (1.0f - (volume / targetVolume)));

	if (targetVolume - volume < 0.001f || t >= 1.0)
	{
		t = 0.0f;
		isDecaying = true;
		volume = -1.0;
		soundPlayed = false;
	}
}

void SoundEmmiter::MoveToPlayer()
{
	GetGameObject()->SetPostion(scene->MainCamera->GetGameObject()->GetPosition() + soundLightOffset);
	scene->Lights[soundLight].Position = scene->MainCamera->GetGameObject()->GetPosition() + soundLightOffset;
}
