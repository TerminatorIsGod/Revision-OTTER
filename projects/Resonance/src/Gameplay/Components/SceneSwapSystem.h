#pragma once
#include "IComponent.h"
#include "Gameplay/Physics/RigidBody.h"
#include "Gameplay/Scene.h"
#include <filesystem>

/// <summary>
/// A simple behaviour that applies an impulse along the Z axis to the 
/// rigidbody of the parent when the space key is pressed
/// </summary>
class SceneSwapSystem : public Gameplay::IComponent {
public:
	typedef std::shared_ptr<SceneSwapSystem> Sptr;

	SceneSwapSystem();
	virtual ~SceneSwapSystem();

	virtual void Awake() override;
	virtual void Update(float deltaTime) override;

	void swapScene(std::string& path);

	void setScene(Gameplay::Scene::Sptr scene);
	void setWindow(GLFWwindow* window);

	Gameplay::Scene::Sptr getScene();

public:
	virtual void RenderImGui() override;
	MAKE_TYPENAME(SceneSwapSystem);
	virtual nlohmann::json ToJson() const override;
	static SceneSwapSystem::Sptr FromJson(const nlohmann::json& blob);

protected:

	Gameplay::Scene::Sptr _scene;
	GLFWwindow* _window;
};