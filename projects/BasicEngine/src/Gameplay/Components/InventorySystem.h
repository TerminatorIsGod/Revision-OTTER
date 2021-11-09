#pragma once
#include "IComponent.h"
#include "Gameplay/Physics/RigidBody.h"

/// <summary>
/// A simple behaviour that applies an impulse along the Z axis to the 
/// rigidbody of the parent when the space key is pressed
/// </summary>
class InventorySystem : public Gameplay::IComponent {
public:
	typedef std::shared_ptr<InventorySystem> Sptr;

	InventorySystem();
	virtual ~InventorySystem();

	virtual void Awake() override;
	virtual void Update(float deltaTime) override;

	int getKeysAmount();
	void setKeysAmount(int keysAmount);
	void addKey();
	bool useKey();

public:
	virtual void RenderImGui() override;
	MAKE_TYPENAME(InventorySystem);
	virtual nlohmann::json ToJson() const override;
	static InventorySystem::Sptr FromJson(const nlohmann::json& blob);

protected:

	int _keys;
};