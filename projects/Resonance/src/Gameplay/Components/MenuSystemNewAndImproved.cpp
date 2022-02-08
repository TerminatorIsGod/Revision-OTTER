#include "Gameplay/Components/InteractSystem.h"
#include <GLFW/glfw3.h>
#include "Gameplay/GameObject.h"
#include "Gameplay/Scene.h"
#include "Utils/ImGuiHelper.h"
#include <Gameplay/Components/MenuSystemNewAndImproved.h>
#include "Gameplay/Components/GUI/RectTransform.h"
void MenuSystemNewAndImproved::Awake()
{
	_scene = GetGameObject()->GetScene();
	_window = _scene->Window;

	int windx, windy;
	glfwGetWindowSize(_window, &windx, &windy);

	centerPos.x = windx / 2;
	centerPos.y = windy / 2;

	auto _ui = GetGameObject()->Get<RectTransform>();
	_ui->SetSize(glm::vec2((windx / 4.0), (windy / 4.0)));

	offscreenPos = glm::vec2(10000, 10000);

	_ui->SetPosition(offscreenPos);
}

void MenuSystemNewAndImproved::RenderImGui() {
	LABEL_LEFT(ImGui::InputInt, "GLFW Key Code", &key, 0);
}

nlohmann::json MenuSystemNewAndImproved::ToJson() const {
	return {
		{ "key", key}
	};
}

MenuSystemNewAndImproved::MenuSystemNewAndImproved() :
	IComponent(),
	key(0)
{ }

MenuSystemNewAndImproved::~MenuSystemNewAndImproved() = default;

MenuSystemNewAndImproved::Sptr MenuSystemNewAndImproved::FromJson(const nlohmann::json & blob) {
	MenuSystemNewAndImproved::Sptr result = std::make_shared<MenuSystemNewAndImproved>();
	result->key = blob["key"];
	return result;
}

void MenuSystemNewAndImproved::Update(float deltaTime) {

	if ((glfwGetKey(_window, key) == GLFW_PRESS) && (isKeyPressed == false)) {
		isKeyPressed = true;
		isToggled = !isToggled;
		ToggleMenu();
	}
	else if ((glfwGetKey(_window, key) != GLFW_PRESS) && (isKeyPressed == true)) {
		isKeyPressed = false;
	}
}

void MenuSystemNewAndImproved::ToggleMenu() {
	auto _ui = GetGameObject()->Get<RectTransform>();

	if (isToggled) {
		_ui->SetPosition(centerPos);
	}
	else {
		_ui->SetPosition(offscreenPos);
	}
}

