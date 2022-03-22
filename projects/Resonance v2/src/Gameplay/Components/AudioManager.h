#pragma once
#include "IComponent.h"
#include "Gameplay/GameObject.h"
#include "Gameplay/Scene.h"
//#include "fmod.hpp"
#include "fmod_studio.hpp"
#include <unordered_map>

using namespace Gameplay;

class AudioManager : public Gameplay::IComponent {
public:
	typedef std::shared_ptr<AudioManager> Sptr;
	FMOD::System* system;
	FMOD::Studio::System* studioSystem;
	FMOD::Studio::Bank* soundBank;
	FMOD::Studio::Bank* soundStringBank;
	FMOD::Studio::EventDescription* soundEvents[20];

	float volume = 1.0f;
	std::string track = "L1_Ambiance";
	AudioManager() = default;
	~AudioManager();
	virtual void Update(float deltaTime) override;
	virtual void Awake() override;

	virtual void RenderImGui() override;
	virtual nlohmann::json ToJson() const override;
	static AudioManager::Sptr FromJson(const nlohmann::json& data);

	void LoadSound(const std::string& soundName, const std::string& filename);
	void UnloadSound(const std::string& soundName);
	void StopAllSounds();

	FMOD::Studio::EventInstance* PlaySoundByName(const std::string& soundName, float vol = 1.0f, glm::vec3 pos = glm::vec3(0.0f), bool lowpass = false);
	//FMOD::Channel* PlaySoundWithVariation(const std::string& soundName, float baseVol = 1.0f, float basePitch = 1.0f, float volRange = 1.0f, float pitchRange = 1.0f, glm::vec3 pos = glm::vec3(0.0f));
	//void PauseSoundByName(const std::string& soundName);
	void PlayFootstepSound(glm::vec3 pos, float vol);


	const FMOD_VECTOR GlmVectorToFmodVector(glm::vec3 vec);
	MAKE_TYPENAME(AudioManager);

protected:
	FMOD::Channel* footstepChannel;
	//std::unordered_map<std::string, FMOD::Sound*> sounds;
	std::unordered_map<std::string, FMOD::Studio::EventDescription*> events;

};