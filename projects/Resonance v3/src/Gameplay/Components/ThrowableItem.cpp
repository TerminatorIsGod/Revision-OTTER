#include "ThrowableItem.h"
#include "Gameplay/Components/SoundEmmiter.h"
#include "Utils/ImGuiHelper.h"
#include "Utils/JsonGlmHelpers.h"
#include "Gameplay/Components/AudioManager.h"
#include "Gameplay/Components/SimpleCameraControl.h"
#include "Application/Application.h"
#include "Utils\GlmBulletConversions.h"


void ThrowableItem::Awake()
{
	scene = GetGameObject()->GetScene();
	player = scene->MainCamera->GetGameObject()->GetParent();
	Application& app = Application::Get();
	_window = app.GetWindow();

	GetComponent<Gameplay::Physics::RigidBody>()->SetAngularDamping(0.5f);
	GetComponent<Gameplay::Physics::RigidBody>()->SetLinearDamping(0.2f);
	GetComponent<Gameplay::Physics::RigidBody>()->SetType(RigidBodyType::Kinematic);

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
				scene->audioManager->Get<AudioManager>()->PlaySoundByName("GlassPickup", 1.0f);
				GetComponent<Gameplay::Physics::RigidBody>()->SetType(RigidBodyType::Unknown);
				GetComponent<Gameplay::Physics::RigidBody>()->SetLinearVelocity(glm::vec3(0.0f));
				thrown = false;
				player->Get<SimpleCameraControl>()->allowInteraction = false;
				isEPressed = true;
			}
		}
		else
			isEPressed = false;
	}
	else if (!thrown && isHeld)
	{
		if (glfwGetKey(_window, GLFW_KEY_E))
		{

			if (!isEPressed)
			{
				isHeld = false;
				GetComponent<Gameplay::Physics::RigidBody>()->SetType(RigidBodyType::Dynamic);
				GetComponent<Gameplay::Physics::RigidBody>()->SetLinearVelocity(glm::vec3(0.0f));
				player->Get<SimpleCameraControl>()->allowInteraction = true;
				isEPressed = true;
			}
		}
		else
			isEPressed = false;
	}

	if (thrown)
	{
		//hit something
		if (prevVel - glm::length(GetComponent<Gameplay::Physics::RigidBody>()->GetLinearVelocity()) > 1.0f)
		{
			GetGameObject()->Get<SoundEmmiter>()->MoveToPos(GetGameObject()->GetPosition());
			GetGameObject()->Get<SoundEmmiter>()->targetVolume = GetGameObject()->Get<SoundEmmiter>()->distractionVolume;
			GetGameObject()->Get<SoundEmmiter>()->isDecaying = false;
			GetGameObject()->Get<SoundEmmiter>()->lerpSpeed = 4.0f;

			//thrown = false;

			if (destroyOnImpact)
			{
				GetGameObject()->SetPostion(glm::vec3(420, 420, -420));
			}
		}
		prevVel = glm::length(GetComponent<Gameplay::Physics::RigidBody>()->GetLinearVelocity());
	}
	else
	{
		GetGameObject()->Get<SoundEmmiter>()->MoveToPos(GetGameObject()->GetPosition());
	}

	if (isHeld)
	{
		player->Get<SimpleCameraControl>()->ShowDropThrow();
		glm::vec4 newOffset = glm::vec4(0.0f, 0.0f, -1.0f, 1.0f);
		glm::vec3 localOffset = glm::vec3(newOffset * player->GetChildren()[0]->GetInverseTransform());

		glm::vec3 offset2 = glm::vec3(0, 0, 780);
		glm::vec4 newOffset2 = glm::vec4(offset2, 1.0);
		glm::vec3 localOffset2 = glm::vec3(newOffset2 * player->GetChildren()[0]->GetInverseTransform());

		//Raycast to make sure player isn't sticking a throwable thru a wall
		btCollisionWorld::ClosestRayResultCallback hit(ToBt(player->GetPosition()), ToBt(player->GetPosition() + player->Get<SimpleCameraControl>()->viewDir * 2.0f));
		scene->GetPhysicsWorld()->rayTest(ToBt(player->GetPosition()), ToBt(player->GetPosition() + player->Get<SimpleCameraControl>()->viewDir * 2.0f), hit);

		glm::vec3 newPos;

		if (!hit.hasHit() || glm::round(GetGameObject()->GetPosition()) == glm::round(ToGlm(hit.m_collisionObject->getWorldTransform().getOrigin())))
			newPos = player->GetPosition() + localOffset * 2.0f;

		if (hit.hasHit() && glm::round(GetGameObject()->GetPosition()) != glm::round(ToGlm(hit.m_collisionObject->getWorldTransform().getOrigin())))
			newPos = player->GetPosition() + localOffset * (glm::length(ToGlm(hit.m_hitPointWorld) - player->GetPosition()));

		GetGameObject()->SetPostion(glm::mix(GetGameObject()->GetPosition(), newPos, deltaTime * 50.0f));
		GetGameObject()->LookAt(player->GetPosition() + localOffset2);

		//Throw
		if (glfwGetMouseButton(_window, GLFW_MOUSE_BUTTON_LEFT))
		{
			isHeld = false;
			thrown = true;
			GetComponent<Gameplay::Physics::RigidBody>()->SetType(RigidBodyType::Dynamic);
			GetComponent<Gameplay::Physics::RigidBody>()->SetLinearVelocity(glm::vec3(0.0f));
			//GetComponent<Gameplay::Physics::RigidBody>()->IsEnabled = true;
			GetComponent<Gameplay::Physics::RigidBody>()->SetLinearVelocity((player->Get<Gameplay::Physics::RigidBody>()->GetLinearVelocity() / 2.0f) + (player->Get<SimpleCameraControl>()->viewDir * 18.0f));
			prevVel = glm::length(GetComponent<Gameplay::Physics::RigidBody>()->GetLinearVelocity()); //Resetting PrevVel for if it has been caught in mid air
			player->Get<SimpleCameraControl>()->allowInteraction = true;
			player->Get<SimpleCameraControl>()->promptShown = false; //Makes sure the ui size is reset so the pickup prompt doesn't look stretched

		}
	}
	else
	{
		//Gravity
		GetComponent<Gameplay::Physics::RigidBody>()->SetLinearVelocity(GetComponent<Gameplay::Physics::RigidBody>()->GetLinearVelocity() + (glm::vec3(0.0f, 0.0f, -12.0f) * deltaTime));
	}

}

void ThrowableItem::RenderImGui() {

	LABEL_LEFT(ImGui::Checkbox, "Destroy On Impact", &destroyOnImpact);

}

nlohmann::json ThrowableItem::ToJson() const {

	nlohmann::json result;
	result["destroyOnImpact"] = destroyOnImpact;

	return result;
}

ThrowableItem::Sptr ThrowableItem::FromJson(const nlohmann::json& blob) {
	ThrowableItem::Sptr result = std::make_shared<ThrowableItem>();
	result->destroyOnImpact = JsonGet(blob, "destroyOnImpact", true);

	return result;
}

