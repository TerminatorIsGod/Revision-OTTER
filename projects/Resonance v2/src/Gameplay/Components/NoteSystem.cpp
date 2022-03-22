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
	_player = GetGameObject()->GetScene()->FindObjectByName("Main Camera");
	_lerpS = GetGameObject()->Get<LerpSystem>();
	Application& app = Application::Get();
	_window = app.GetWindow();

}

void NoteSystem::RenderImGui() {
	LABEL_LEFT(ImGui::DragFloat, "Distance", &_distance, 1.0f);
	LABEL_LEFT(ImGui::DragFloat, "Interact Distance", &_interactDistance, 1.0f);
}

nlohmann::json NoteSystem::ToJson() const {
	return {
		{ "Distance", _distance},
		{ "InteractDistance", _interactDistance },
		{ "NoteObjName", noteName }
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
	return result;
}

void NoteSystem::Update(float deltaTime) {

	glm::vec3 ppos = _player->GetPosition();
	glm::vec3 opos = GetGameObject()->GetPosition();

	glm::vec3 tpos = ppos - opos;
	_distance = sqrt(pow(tpos.x, 2) + pow(tpos.y, 2) + pow(tpos.z, 2));


	//Animated Objects (raycast based)
	if (_player->Get<SimpleCameraControl>()->interactionObjectPos == opos && !_player->Get<SimpleCameraControl>()->promptShown) {

		//show interact message
		_player->Get<SimpleCameraControl>()->ShowPickup();
	}

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

	Application& app = Application::Get();

	if (!app.isGamePaused && isOpen)
		interact();

}

void NoteSystem::interact() {

	_distance = (_player->GetPosition().length() - GetGameObject()->GetPosition().length());

	glm::vec3 ppos = _player->GetPosition();
	glm::vec3 opos = GetGameObject()->GetPosition();

	Application& app = Application::Get();

	glm::vec3 tpos = ppos - opos;
	_distance = sqrt(pow(tpos.x, 2) + pow(tpos.y, 2) + pow(tpos.z, 2));

	//Animated Object (based on raycast)
	if (_distance <= _interactDistance && _player->Get<SimpleCameraControl>()->interactionObjectPos == opos) { //_player->Get<SimpleCameraControl>()->interactionObjectPos == opos
		int windx, windy;
		glfwGetWindowSize(_window, &windx, &windy);

		if (isOpen)
		{
			isOpen = false;
			GetGameObject()->GetScene()->audioManager->Get<AudioManager>()->PlaySoundByName("NotePutdown", 1.0f);
			GetGameObject()->GetScene()->FindObjectByName(noteName)->Get<GuiPanel>()->IsEnabled = false;
			std::cout << "Note disabled\n\n";
			//if (GetGameObject()->GetScene()->FindObjectByName("Main Camera"))
				//GetGameObject()->GetScene()->FindObjectByName("Main Camera")->Get<SimpleCameraControl>()->IsEnabled = true;
			//app.isGamePaused = false;
		}
		else
		{
			isOpen = true;

			GetGameObject()->GetScene()->audioManager->Get<AudioManager>()->PlaySoundByName("NotePickup", 1.0f);
			GetGameObject()->GetScene()->FindObjectByName(noteName)->Get<GuiPanel>()->IsEnabled = true;
			GetGameObject()->GetScene()->FindObjectByName(noteName)->Get<RectTransform>()->SetPosition(glm::vec2(windx / 2, windy / 2));
			GetGameObject()->GetScene()->FindObjectByName(noteName)->Get<RectTransform>()->SetSize(glm::vec2((windx / 4), (windy / 4.5)));
			std::cout << "Note enabled\n\n";
			app.isGamePaused = true;
			app.isInteracting = true;
		}
	}

}

