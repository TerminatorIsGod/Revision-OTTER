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
	
	float endx = 0;
	float endy = 0;
	float endz = 0;

	float oldCheck = 0;

	float tLength = 1;

	glm::quat startRot = glm::quat(1,0,0,0);
	glm::quat endRot = glm::quat(1, 0, 0, 0);

	float t = 0;
	bool beginLerp = false;
	bool lerpReverse = false;


	glm::vec3 lerpstuff(glm::vec3 a, glm::vec3 b, float t);

	void setRotationStart();
	void setRotationStart(glm::vec3 xyz);
	void setRotationEnd();
	void setRotationEnd(glm::vec3 xyz);

protected:

	
};