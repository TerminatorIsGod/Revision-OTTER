#pragma once
#include "IComponent.h"
#include "Gameplay/GameObject.h"
#include "Gameplay/Scene.h"
#include "Gameplay/Components/RenderComponent.h"

using namespace Gameplay;

/// <summary>
/// Showcases a very simple behaviour that rotates the parent gameobject at a fixed rate over time
/// </summary>
class ThrowableItem : public Gameplay::IComponent {
public:
	typedef std::shared_ptr<ThrowableItem> Sptr;

	ThrowableItem() = default;

	//Properties
	Scene* scene;
	GameObject* player;



	virtual void Awake() override;
	virtual void Update(float deltaTime) override;
	virtual void RenderImGui() override;

	virtual nlohmann::json ToJson() const override;
	static ThrowableItem::Sptr FromJson(const nlohmann::json& data);

	MAKE_TYPENAME(ThrowableItem);

protected:

};