#pragma once
#include "IComponent.h"
#include "Gameplay/Physics/RigidBody.h"

/// <summary>
/// A simple behaviour that applies an impulse along the Z axis to the 
/// rigidbody of the parent when the space key is pressed
/// </summary>
class LerpSystem : public Gameplay::IComponent {
public:
	typedef std::shared_ptr<LerpSystem> Sptr;

	LerpSystem();
	virtual ~LerpSystem();

	virtual void Awake() override;
	virtual void Update(float deltaTime) override;

	int getKeysAmount();
	void setKey(int key, bool value);
	bool getKey(int key);

public:
	virtual void RenderImGui() override;
	MAKE_TYPENAME(LerpSystem);
	virtual nlohmann::json ToJson() const override;
	static LerpSystem::Sptr FromJson(const nlohmann::json& blob);

	float startx = 0;
	float starty = 0;
	float startz = 0;
	
	float endx;
	float endy;
	float endz;

	float tLength;

	glm::quat startRot;
	glm::quat endRot;

	float t;
	bool beginLerp;
	bool lerpReverse;

	void setRotationStart();
	void setRotationStart(glm::vec3 xyz);
	void setRotationEnd();
	void setRotationEnd(glm::vec3 xyz);

protected:

	
};