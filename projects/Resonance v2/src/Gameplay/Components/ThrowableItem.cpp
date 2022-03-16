#include "ThrowableItem.h"
#include "Gameplay/Components/SoundEmmiter.h"
#include "Utils/ImGuiHelper.h"
#include "Utils/JsonGlmHelpers.h"
#include "Gameplay/Components/AudioManager.h"
#include "Gameplay/Components/SimpleCameraControl.h"
#include "Application/Application.h"


void ThrowableItem::Awake()
{
	scene = GetGameObject()->GetScene();
	player = scene->MainCamera->GetGameObject();
	Application& app = Application::Get();
	_window = app.GetWindow();

	GetComponent<Gameplay::Physics::RigidBody>()->SetAngularDamping(0.5f);
	GetComponent<Gameplay::Physics::RigidBody>()->SetLinearDamping(0.2f);

	GetGameObject()->Get<SoundEmmiter>()->isThrowable = true;
}

void ThrowableItem::Update(float deltaTime)
{
	if (!isHeld && glm::round(player->Get<SimpleCameraControl>()->interactionObjectPos) == glm::round(GetGameObject()->GetPosition()))
	{
		//Show pick up prompt
		player->Get<SimpleCameraControl>()->ShowPickup();

		if (glfwGetKey(_window, GLFW_KEY_E))
		{
			if (!isEPressed)
			{
				isHeld = true;
				GetComponent<Gameplay::Physics::RigidBody>()->SetType(RigidBodyType::Kinematic);
				GetComponent<Gameplay::Physics::RigidBody>()->SetLinearVelocity(glm::vec3(0.0f));
				thrown = false;
				//GetComponent<Gameplay::Physics::RigidBody>()->IsEnabled = false;
				isEPressed = true;
			}
		}
		else
			isEPressed = false;
	}
	else if (!thrown)
	{
		if (glfwGetKey(_window, GLFW_KEY_E))
		{
			if (!isEPressed)
			{
				isHeld = false;
				GetComponent<Gameplay::Physics::RigidBody>()->SetType(RigidBodyType::Dynamic);
				GetComponent<Gameplay::Physics::RigidBody>()->SetLinearVelocity(glm::vec3(0.0f));
				//GetComponent<Gameplay::Physics::RigidBody>()->IsEnabled = true;
				isEPressed = true;
			}
		}
		else
			isEPressed = false;
	}

	if (thrown)
	{
		//std::cout << "\nVEL Dif " << prevVel - glm::length(GetComponent<Gameplay::Physics::RigidBody>()->GetLinearVelocity());
		if (prevVel - glm::length(GetComponent<Gameplay::Physics::RigidBody>()->GetLinearVelocity()) > 1.0f)
		{
			GetGameObject()->Get<SoundEmmiter>()->MoveToPos(GetGameObject()->GetPosition());
			GetGameObject()->Get<SoundEmmiter>()->targetVolume = GetGameObject()->Get<SoundEmmiter>()->distractionVolume;
			GetGameObject()->Get<SoundEmmiter>()->isDecaying = false;
			GetGameObject()->Get<SoundEmmiter>()->lerpSpeed = 4.0f;
			thrown = false;
		}
		prevVel = glm::length(GetComponent<Gameplay::Physics::RigidBody>()->GetLinearVelocity());
	}

	if (isHeld)
	{
		glm::vec4 newOffset = glm::vec4(0.0f, 0.0f, -1.0f, 1.0f);
		glm::vec3 localOffset = glm::vec3(newOffset * player->GetInverseTransform());

		glm::vec3 offset2 = glm::vec3(0, 0, 780);
		glm::vec4 newOffset2 = glm::vec4(offset2, 1.0);
		glm::vec3 localOffset2 = glm::vec3(newOffset2 * player->GetInverseTransform());

		GetGameObject()->SetPostion(player->GetPosition() + localOffset * 2.0f);
		GetGameObject()->LookAt(player->GetPosition() + localOffset2);

		//Throw
		if (glfwGetMouseButton(_window, GLFW_MOUSE_BUTTON_LEFT))
		{
			//std::cout << "\nLEFT CLICK";
			isHeld = false;
			thrown = true;
			GetComponent<Gameplay::Physics::RigidBody>()->SetType(RigidBodyType::Dynamic);
			GetComponent<Gameplay::Physics::RigidBody>()->SetLinearVelocity(glm::vec3(0.0f));
			//GetComponent<Gameplay::Physics::RigidBody>()->IsEnabled = true;
			GetComponent<Gameplay::Physics::RigidBody>()->ApplyForce(player->Get<SimpleCameraControl>()->viewDir * 850.0f);
		}
	}
	else
	{
		//Gravity
		GetComponent<Gameplay::Physics::RigidBody>()->ApplyForce(glm::vec3(0.0f, 0.0f, -12.0f));
	}

}

void ThrowableItem::RenderImGui() {

	LABEL_LEFT(ImGui::DragFloat3, "temp", &temp.x);

}

nlohmann::json ThrowableItem::ToJson() const {

	nlohmann::json result;
	result["temp"] = temp;
	return result;

}

ThrowableItem::Sptr ThrowableItem::FromJson(const nlohmann::json& blob) {
	ThrowableItem::Sptr result = std::make_shared<ThrowableItem>();
	result->temp = JsonGet(blob, "temp", glm::vec3(0.0f));

	return result;
}

