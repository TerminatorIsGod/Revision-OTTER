#include "Gameplay/Components/InteractSystem.h"
#include <GLFW/glfw3.h>
#include "Gameplay/GameObject.h"
#include "Gameplay/Scene.h"
#include "Utils/ImGuiHelper.h"
#include <Gameplay/Components/MenuSystemNewAndImproved.h>
#include "Gameplay/Components/GUI/RectTransform.h"
#include "Application/Application.h"

void MenuSystemNewAndImproved::Awake()
{
	_scene = GetGameObject()->GetScene();
	Application& app = Application::Get();
	_window = app.GetWindow();

	int windx, windy;
	glfwGetWindowSize(_window, &windx, &windy);

	centerPos.x = windx / 2;
	centerPos.y = windy / 2;

	auto _ui = GetGameObject()->Get<RectTransform>();
	_ui->SetSize(glm::vec2((windx / 4.0), (windy / 4.0)));

	offscreenPos = glm::vec2(10000, 10000);

	_ui->SetPosition(offscreenPos);

	lastWindowSize = glm::vec2(windx, windy);
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


	isPauseScreen();

	if ((glfwGetKey(_window, key) == GLFW_PRESS) && (isKeyPressed == false)) {
		isKeyPressed = true;
		isToggled = !isToggled;

		int windx, windy;
		glfwGetWindowSize(_window, &windx, &windy);

		if (lastWindowSize != glm::vec2(windx, windy)) {

			centerPos.x = windx / 2;
			centerPos.y = windy / 2;

			auto _ui = GetGameObject()->Get<RectTransform>();
			_ui->SetSize(glm::vec2((windx / 4.0), (windy / 4.0)));

			offscreenPos = glm::vec2(10000, 10000);

			_ui->SetPosition(offscreenPos);

			lastWindowSize = glm::vec2(windx, windy);

		}

		ToggleMenu();
	}
	else if ((glfwGetKey(_window, key) != GLFW_PRESS) && (isKeyPressed == true)) {
		isKeyPressed = false;
	}
}

void MenuSystemNewAndImproved::isPauseScreen() {
	if (GetGameObject()->Name == "PauseScreen") { // || GetGameObject()->Get<GuiPanel>()->IsEnabled

		int windx, windy;
		glfwGetWindowSize(_window, &windx, &windy);

		if (lastWindowSize != glm::vec2(windx, windy)) {

			centerPos.x = windx / 2;
			centerPos.y = windy / 2;

			auto _ui = GetGameObject()->Get<RectTransform>();
			_ui->SetSize(glm::vec2((windx / 4.0), (windy / 4.0)));

			offscreenPos = glm::vec2(10000, 10000);

			_ui->SetPosition(offscreenPos);

			lastWindowSize = glm::vec2(windx, windy);

		}
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

