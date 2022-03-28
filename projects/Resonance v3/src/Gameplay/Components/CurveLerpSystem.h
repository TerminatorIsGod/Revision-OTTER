#pragma once
#include "IComponent.h"
#include <GLM/gtc/quaternion.hpp>

/// <summary>
/// A simple behaviour that applies an impulse along the Z axis to the 
/// rigidbody of the parent when the space key is pressed
/// </summary>
class CurveLerpSystem : public Gameplay::IComponent {
public:
	typedef std::shared_ptr<CurveLerpSystem> Sptr;

	CurveLerpSystem();
	virtual ~CurveLerpSystem();

	virtual void Awake() override;
	virtual void Update(float deltaTime) override;

public:
	virtual void RenderImGui() override;
	MAKE_TYPENAME(CurveLerpSystem);
	virtual nlohmann::json ToJson() const override;
	static CurveLerpSystem::Sptr FromJson(const nlohmann::json& blob);

	int segment = 0;
	std::vector<glm::vec3> points = {
		glm::vec3(3.5f, 3.25f, -12.0f),
		glm::vec3(-26, 48, -12.0f),
		glm::vec3(-23, 6, -12.0f),
		glm::vec3(24, 32, -12.0f)
	};

	float tLength = 1;

	glm::quat startRot = glm::quat(1, 0, 0, 0);
	glm::quat endRot = glm::quat(1, 0, 0, 0);

	float t = 0;

protected:


};