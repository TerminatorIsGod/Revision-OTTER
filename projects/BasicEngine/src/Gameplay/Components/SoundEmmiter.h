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
	float volume = 0;
	float targetVolume = 0;
	float decaySpeed = 1;
	float attackSpeed = 4;
	float lerpSpeed;

	Scene* scene;
	MeshResource::Sptr soundRingMesh = ResourceManager::CreateAsset<MeshResource>("soundRing.obj");
	Material::Sptr soundRingMat;

	bool isDecaying = true;
	bool muteAtZero = false;


	//Functions
	void Decay(float deltaTime);
	void Attack(float deltaTime);

	//Generic Functions
	glm::vec3 speed = glm::vec3(0.0f);

	virtual void Awake() override;

	virtual void Update(float deltaTime) override;

	virtual void RenderImGui() override;

	virtual nlohmann::json ToJson() const override;
	static SoundEmmiter::Sptr FromJson(const nlohmann::json& data);

	MAKE_TYPENAME(SoundEmmiter);

protected:
	GameObject::Sptr soundRing;
	glm::vec3 soundRingOffset = glm::vec3(0, 0, -5.0f);

};