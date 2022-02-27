#pragma once
#include "IComponent.h"
#include "Gameplay/GameObject.h"
#include "Gameplay/Scene.h"
#include "Gameplay/Components/RenderComponent.h"

using namespace Gameplay;

/// <summary>
/// Showcases a very simple behaviour that rotates the parent gameobject at a fixed rate over time
/// </summary>
class SoundEmmiter : public Gameplay::IComponent {
public:
	typedef std::shared_ptr<SoundEmmiter> Sptr;

	SoundEmmiter() = default;
	//Hey, ur probably gonna have to make sure shared pointers get deleted properly so no rings persist during scene changes
	//~SoundEmmiter(); 

	//Properties
	float volume = 0.0f;
	float targetVolume = 0.0f;
	float distractionVolume = 100;
	float decaySpeed = 1;
	float attackSpeed = 4;
	float lerpSpeed;
	glm::vec3 soundLightOffset = glm::vec3(0, 0, 0);
	Scene* scene;

	bool isDecaying = true;
	bool muteAtZero = false;
	bool isPlayerLight = false;
	bool linearLerp = false;

	glm::vec3 defaultColour = glm::vec3(0.03f, 0.03f, 0.03f);
	glm::vec3 colour = defaultColour;
	float t = 0.0f;
	//Functions
	void Decay(float deltaTime);
	void Attack(float deltaTime);
	void MoveToPlayer();
	//Generic Functions
	glm::vec3 speed = glm::vec3(0.0f);
	std::string soundName;
	float soundVol;


	virtual void Awake() override;

	virtual void Update(float deltaTime) override;

	virtual void RenderImGui() override;

	virtual nlohmann::json ToJson() const override;
	static SoundEmmiter::Sptr FromJson(const nlohmann::json& data);

	MAKE_TYPENAME(SoundEmmiter);

protected:
	int soundLight;
	bool soundPlayed;
};