#include "Gameplay/Components/SoundEmmiter.h"
#include "Utils/ImGuiHelper.h"
#include "Utils/JsonGlmHelpers.h"


void SoundEmmiter::Awake()
{
	lerpSpeed = attackSpeed;

	scene = GetGameObject()->GetScene();
	soundRing = scene->CreateGameObject("Sound Ring");
	{
		// Set position in the scene
		soundRing->SetPostion(GetGameObject()->GetPosition());

		//Create and attach a render component
		RenderComponent::Sptr renderer = soundRing->Add<RenderComponent>();
		renderer->SetMesh(soundRingMesh);
		renderer->SetMaterial(soundRingMat);
	}

	scene->soundEmmiters.push_back(soundRing);
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

	soundRing->SetScale(glm::vec3(volume));
	soundRing->SetPostion(GetGameObject()->GetPosition() + soundRingOffset);
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
