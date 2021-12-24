#pragma once
#include "IComponent.h"
#include "Gameplay/GameObject.h"
#include "Gameplay/Scene.h"
using namespace Gameplay;

/// <summary>
/// Showcases a very simple behaviour that rotates the parent gameobject at a fixed rate over time
/// </summary>
class UIElement : public Gameplay::IComponent {
public:
	typedef std::shared_ptr<UIElement> Sptr;

	UIElement() = default;

	glm::vec3 posOffset = glm::vec3(0.0f);

	enum Type
	{
		General,
		Prompt,
		Meter
	};

	Type uiType = General;

	virtual void Awake() override;

	virtual void RenderImGui() override;
	virtual nlohmann::json ToJson() const override;
	static UIElement::Sptr FromJson(const nlohmann::json& data);

	MAKE_TYPENAME(UIElement);


};
