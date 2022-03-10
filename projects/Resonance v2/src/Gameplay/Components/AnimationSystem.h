#pragma once
#include "IComponent.h"
#include "Gameplay/Physics/RigidBody.h"
#include "Gameplay/MeshResource.h"

/// <summary>
/// A simple behaviour that applies an impulse along the Z axis to the 
/// rigidbody of the parent when the space key is pressed
/// </summary>
class AnimationSystem : public Gameplay::IComponent {
public:
	typedef std::shared_ptr<AnimationSystem> Sptr;

	AnimationSystem();
	virtual ~AnimationSystem();

	virtual void Awake() override;
	virtual void Update(float deltaTime) override;


public:
	virtual void RenderImGui() override;
	MAKE_TYPENAME(AnimationSystem);
	virtual nlohmann::json ToJson() const override;
	static AnimationSystem::Sptr FromJson(const nlohmann::json& blob);

	bool isMeshesLoaded = false;

	bool showInfo = false;
	bool listMeshes = false;

	int curFrame = 1;
	float speed = 0.0f;
	float timer = 0.0f;

	//meshes vector
	std::vector<Gameplay::MeshResource::Sptr> meshes;

	int curMesh = 0;

	std::string filePrefix = "";
	int frames = 0;

protected:

	
};