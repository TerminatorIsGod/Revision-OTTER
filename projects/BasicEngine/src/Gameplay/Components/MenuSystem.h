#pragma once
#include "IComponent.h"
#include "Gameplay/Physics/RigidBody.h"
#include "Gameplay/Scene.h"
#include "Gameplay/GameObject.h"
#include "Gameplay/Material.h"
#include <Gameplay/Components/RenderComponent.h>

/// <summary>
/// A simple behaviour that applies an impulse along the Z axis to the 
/// rigidbody of the parent when the space key is pressed
/// </summary>
namespace Gameplay {
	class MenuSystem : public Gameplay::IComponent {
	public:
		typedef std::shared_ptr<MenuSystem> Sptr;

		MenuSystem();
		virtual ~MenuSystem();

		virtual void Awake() override;
		virtual void Update(float deltaTime) override;

		Camera::Sptr getMenuCamera();
		void mainScene(Scene::Sptr scene);
		void setPauseMaterial(Scene::Sptr scene);
		void setMenuMaterial(Scene::Sptr scene);
		void createCamera();
		void createObjects();

	public:
		virtual void RenderImGui() override;
		MAKE_TYPENAME(MenuSystem);
		virtual nlohmann::json ToJson() const override;
		static MenuSystem::Sptr FromJson(const nlohmann::json& blob);

		Scene::Sptr _mainScene;
		Camera::Sptr _menucamera;
		Gameplay::Material::Sptr _materialMain;
		Gameplay::Material::Sptr _materialPause;

	protected:
		
	};
}