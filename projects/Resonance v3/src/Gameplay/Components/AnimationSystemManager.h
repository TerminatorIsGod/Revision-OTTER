#pragma once
#include "IComponent.h"
#include "Gameplay/Physics/RigidBody.h"
#include "Gameplay/MeshResource.h"
#include <thread>

/// <summary>
/// A simple behaviour that applies an impulse along the Z axis to the 
/// rigidbody of the parent when the space key is pressed
/// </summary>
class AnimationSystemManager : public Gameplay::IComponent {
public:
	typedef std::shared_ptr<AnimationSystemManager> Sptr;

	AnimationSystemManager();
	virtual ~AnimationSystemManager();

	virtual void Awake() override;
	virtual void Update(float deltaTime) override;

	void loadAnims();


public:
	virtual void RenderImGui() override;
	MAKE_TYPENAME(AnimationSystemManager);
	virtual nlohmann::json ToJson() const override;
	static AnimationSystemManager::Sptr FromJson(const nlohmann::json& blob);

	bool isMeshesLoaded = false;

	bool showInfo = false;
	bool listMeshes = false;

	//meshes vector
	std::vector<Gameplay::MeshResource::Sptr> meshes;

	std::string filePrefix = "";
	int frames = 0;

	bool isLoaded = false;

protected:

	
};