#include "Gameplay/Components/Ladder.h"
#include "Utils/ImGuiHelper.h"
#include "Utils/JsonGlmHelpers.h"
#include "Gameplay/Components/SimpleCameraControl.h"
#include "Gameplay/Components/AudioManager.h"
#include "Application/Application.h"

#include <GLFW/glfw3.h>

void Ladder::Awake()
{
	_scene = GetGameObject()->GetScene();
	Application& app = Application::Get();
	_window = app.GetWindow();
}

void Ladder::Update(float deltaTime)
{
	SimpleCameraControl::Sptr player = _scene->MainCamera->GetGameObject()->Get<SimpleCameraControl>();
	if (player->interactionObjectPos != GetGameObject()->GetPosition())
		return;

	if (!toLevel2)
	{
		//Ui Prompt
		player->ShowClimb();

		if (glfwGetKey(_window, GLFW_KEY_E))
		{
			if (!isEPressed)
			{
				_scene->audioManager->Get<AudioManager>()->PlaySoundByName("LadderClimb");
				player->GetGameObject()->SetPostion(teleportPos);
				player->baseHeight = teleportPos.z;
				isEPressed = true;
			}
		}
		else
			isEPressed = false;
	}
	else
	{
		player->ShowOpen();

		if (glfwGetKey(_window, GLFW_KEY_E))
		{
			if (!isEPressed)
			{
				//Show loading screen here
				Application& app = Application::Get();

				app.LoadScene("level2.json");
				isEPressed = true;
			}
		}
		else
			isEPressed = false;

	}
}

void Ladder::RenderImGui() {
	LABEL_LEFT(ImGui::DragFloat3, "Teleport Position", &teleportPos.x);
	LABEL_LEFT(ImGui::Checkbox, "To Level 2?", &toLevel2);
}

nlohmann::json Ladder::ToJson() const {
	return {
		{ "Teleport Position", teleportPos },
		{ "LevelTransition", toLevel2 }
	};
}

Ladder::Sptr Ladder::FromJson(const nlohmann::json& blob) {
	Ladder::Sptr result = std::make_shared<Ladder>();
	result->teleportPos = JsonGet(blob, "Teleport Position", result->teleportPos);
	result->toLevel2 = JsonGet(blob, "LevelTransition", result->toLevel2);

	return result;
}
