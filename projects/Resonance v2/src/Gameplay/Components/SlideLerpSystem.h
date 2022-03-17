#pragma once
#include "IComponent.h"
#include "Gameplay/Physics/RigidBody.h"

/// <summary>
/// A simple behaviour that applies an impulse along the Z axis to the 
/// rigidbody of the parent when the space key is pressed
/// </summary>
class SlideLerpSystem : public Gameplay::IComponent {
public:
	typedef std::shared_ptr<SlideLerpSystem> Sptr;

	SlideLerpSystem();
	virtual ~SlideLerpSystem();

	virtual void Awake() override;
	virtual void Update(float deltaTime) override;

	int getKeysAmount();
	void setKey(int key, bool value);
	bool getKey(int key);

public:
	virtual void RenderImGui() override;
	MAKE_TYPENAME(SlideLerpSystem);
	virtual nlohmann::json ToJson() const override;
	static SlideLerpSystem::Sptr FromJson(const nlohmann::json& blob);

	float startx = 0;
	float starty = 0;
	float startz = 0;
	
	float endx = 0;
	float endy = 0;
	float endz = 0;

	float tLength = 1;

	glm::quat startRot = glm::quat(1,0,0,0);
	glm::quat endRot = glm::quat(1, 0, 0, 0);

	float t = 0;
	bool beginLerp = false;
	bool lerpReverse = false;

	bool doUpdateNbors = false;


	glm::vec3 lerpstuff(glm::vec3 a, glm::vec3 b, float t);

protected:

	
};