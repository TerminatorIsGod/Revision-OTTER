#include "Gameplay/Components/NoteSystem.h"
#include <GLFW/glfw3.h>
#include "Gameplay/GameObject.h"
#include "Gameplay/Scene.h"
#include "Utils/ImGuiHelper.h"
#include <Gameplay/Components/InventorySystem.h>
#include "Gameplay/Components/SimpleCameraControl.h"
#include "Gameplay/InputEngine.h"
#include "Application/Application.h"
#include "Gameplay/Components/AudioManager.h"
#include "../../../../Resonance/src/Gameplay/Components/GUI/GuiPanel.h"
#include "Gameplay/Components/AudioManager.h"

void NoteSystem::Awake()
{
	_player = GetGameObject()->GetScene()->MainCamera->GetGameObject()->GetParent();
	_lerpS = GetGameObject()->Get<LerpSystem>();
	Application& app = Application::Get();
	_window = app.GetWindow();

}

void NoteSystem::RenderImGui() {
	LABEL_LEFT(ImGui::DragFloat, "Distance", &_distance, 1.0f);
	LABEL_LEFT(ImGui::DragFloat, "Interact Distance", &_interactDistance, 1.0f);
	LABEL_LEFT(ImGui::Checkbox, "Is Terminal?", &isTerminal);
}

nlohmann::json NoteSystem::ToJson() const {
	return {
		{ "Distance", _distance},
		{ "InteractDistance", _interactDistance },
		{ "NoteObjName", noteName },
		{ "IsTerminal", isTerminal}
	};
}

NoteSystem::NoteSystem() :
	IComponent(),
	_interactDistance(0),
	_distance(0),
	noteName("")
{ }

NoteSystem::~NoteSystem() = default;

NoteSystem::Sptr NoteSystem::FromJson(const nlohmann::json & blob) {
	NoteSystem::Sptr result = std::make_shared<NoteSystem>();
	result->_distance = blob["Distance"];
	result->_interactDistance = blob["InteractDistance"];
	result->noteName = blob["NoteObjName"];
	result->isTerminal = JsonGet(blob, "IsTerminal", result->isTerminal);

	return result;
}

void NoteSystem::Update(float deltaTime) {

	glm::vec3 opos = GetGameObject()->GetPosition();
	Application& app = Application::Get();

	if (app.isInteracting && isOpen)
	{
		//Moved pausing here so the pause happens a frame after interaction
		//This gives the AudioSystem a frame to play the Pick-Up SFX
		app.isGamePaused = true;
	}

	//Animated Objects (raycast based)
	if (_player->Get<SimpleCameraControl>()->interactionObjectPos == opos) {

		//show interact message
		_player->Get<SimpleCameraControl>()->ShowRead();

		if (glfwGetKey(_window, GLFW_KEY_E)) {
			if (!isKeyPressed)
			{
				std::cout << "Player interacted with note\n\n";
				interact();
				isKeyPressed = true;
			}
		}
		else
		{
			isKeyPressed = false;
		}
	}

	if (!app.isInteracting && !app.isGamePaused && isOpen)
		interact();
}

void NoteSystem::interact() {

	int windx, windy;
	glfwGetWindowSize(_window, &windx, &windy);
	Application& app = Application::Get();

	if (isOpen)
	{
		isOpen = false;
		if (!isTerminal)
			GetGameObject()->GetScene()->audioManager->Get<AudioManager>()->PlaySoundByName("NotePutdown", 1.0f);
		else
			GetGameObject()->GetScene()->audioManager->Get<AudioManager>()->PlaySoundByName("LoggedOut", 1.0f);

		GetGameObject()->GetScene()->FindObjectByName(noteName)->Get<GuiPanel>()->IsEnabled = false;
		std::cout << "Note disabled\n\n";
		//if (GetGameObject()->GetScene()->FindObjectByName("Main Camera"))
			//GetGameObject()->GetScene()->FindObjectByName("Main Camera")->Get<SimpleCameraControl>()->IsEnabled = true;
		//app.isGamePaused = false;
	}
	else
	{
		isOpen = true;
		if (!isTerminal)
			GetGameObject()->GetScene()->audioManager->Get<AudioManager>()->PlaySoundByName("NotePickup", 1.0f);
		else
			GetGameObject()->GetScene()->audioManager->Get<AudioManager>()->PlaySoundByName("LoggedIn", 1.0f);

		GetGameObject()->GetScene()->FindObjectByName(noteName)->Get<GuiPanel>()->IsEnabled = true;
		GetGameObject()->GetScene()->FindObjectByName(noteName)->Get<RectTransform>()->SetSize(glm::vec2((windx / 4), (windy / 4.5)));

		//Can't Change The Position of image as rect tranforms control the position of their child. 
		//glm::vec2 sizeAdj = -glm::vec2(1920, 1080) / 2.0f;
		//GetGameObject()->GetScene()->FindObjectByName(noteName)->Get<RectTransform>()->SetPosition(glm::vec2(windx / 2, windy / 2) + sizeAdj);

		std::cout << "Note enabled\n\n";
		app.isInteracting = true;
	}
}

