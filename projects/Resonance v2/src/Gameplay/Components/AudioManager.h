#pragma once
#include "IComponent.h"
#include "Gameplay/GameObject.h"
#include "Gameplay/Scene.h"
#include "fmod.hpp"
#include <unordered_map>

using namespace Gameplay;

class AudioManager : public Gameplay::IComponent {
public:
	typedef std::shared_ptr<AudioManager> Sptr;

	float volume = 1.0f;

	AudioManager() = default;
	~AudioManager();
	virtual void Update(float deltaTime) override;
	virtual void Awake() override;

	virtual void RenderImGui() override;
	virtual nlohmann::json ToJson() const override;
	static AudioManager::Sptr FromJson(const nlohmann::json& data);

	void LoadSound(const std::string& soundName, const std::string& filename, bool b3d, bool bLooping = false, bool bStream = false);
	void UnloadSound(const std::string& soundName);
	void PlaySoundByName(const std::string& soundName);
	const FMOD_VECTOR GlmVectorToFmodVector(glm::vec3 vec);
	MAKE_TYPENAME(AudioManager);

protected:
	FMOD::System* system;
	std::unordered_map<std::string, FMOD::Sound*> sounds;
};