#include "Gameplay/Components/AudioManager.h"
#include "Utils/ImGuiHelper.h"
#include "Utils/JsonGlmHelpers.h"
#include "Gameplay/Components/SimpleCameraControl.h"
#include "Application/Application.h"

void AudioManager::Awake() {
	GetGameObject()->GetScene()->audioManager = GetGameObject();

	FMOD_RESULT result;
	result = FMOD::Studio::System::create(&studioSystem); // Create the Studio System object.

	// Initialize FMOD Studio, which will also initialize FMOD Low Level
	result = studioSystem->initialize(512, FMOD_STUDIO_INIT_NORMAL, FMOD_INIT_NORMAL, 0);
	studioSystem->getCoreSystem(&system);

	studioSystem->loadBankFile("Audio/Fmod Studio Projects/Resonance Audio/Build/Desktop/Master.bank", FMOD_STUDIO_LOAD_BANK_NORMAL, &soundBank);
	studioSystem->loadBankFile("Audio/Fmod Studio Projects/Resonance Audio/Build/Desktop/Master.strings.bank", FMOD_STUDIO_LOAD_BANK_NORMAL, &soundStringBank);
	soundBank->loadSampleData();

	LoadSound("L1_Ambiance", "event:/Level 1 Ambiance");
	LoadSound("Title", "event:/Title Screen");
	LoadSound("Transition", "event:/Loading Transition");
	LoadSound("Footstep", "event:/Footstep");
	LoadSound("LeaflingPatrol", "event:/Leafling Patrol");
	LoadSound("LeaflingDistracted", "event:/Leafling Distracted");
	LoadSound("LeaflingAgro", "event:/Leafling Aggravated");
	LoadSound("Engines", "event:/Engine Whirring");
	LoadSound("OxygenRefill", "event:/Replenish Oxygen");
	LoadSound("OutOfBreath", "event:/Out of Breath");
	LoadSound("HoldingBreath", "event:/Hold Breath");
	LoadSound("BreathOut", "event:/Release Breath");
	LoadSound("ValveTwist", "event:/Valve Twist");
	LoadSound("VendingMachine", "event:/Vending Machine");
	LoadSound("IdleIn", "event:/Idle Breathe In");
	LoadSound("IdleOut", "event:/Idle Breathe Out");
	LoadSound("StopReplenish", "event:/Stop Replenish Oxygen");
	LoadSound("Death", "event:/Death");
	LoadSound("LadderClimb", "event:/Ladder Climb");
	LoadSound("KeyPickup", "event:/Key Pickup");
	LoadSound("DoorOpen", "event:/Door Open");
	LoadSound("DoorClose", "event:/Door Close");
	LoadSound("Gasp", "event:/Gasp");
	LoadSound("GlassShatter", "event:/Glass Shatter");
	LoadSound("GlassPickup", "event:/Glass Pickup");
	LoadSound("NotePickup", "event:/Note Pickup");
	LoadSound("NotePutdown", "event:/Note Putdown");
	LoadSound("ClockTick", "event:/Clock Tick");
	LoadSound("SafeRoom", "event:/Safe Room");
	LoadSound("LoggedIn", "event:/Logged In");
	LoadSound("LoggedOut", "event:/Logged Out");
	LoadSound("SirenPatrol", "event:/Siren Patrolling");
	LoadSound("SirenDistracted", "event:/Siren Distracted");
	LoadSound("SirenAgro", "event:/Siren Aggravated");
	LoadSound("SirenAlerted", "event:/Siren Alerted");
	LoadSound("TheLabs", "event:/The Labs");
	LoadSound("Generator", "event:/Generator");
	LoadSound("Message", "event:/Emergency Message");
	LoadSound("Echos", "event:/Echos");


	if (track == "L1_Ambiance")
	{
		PlaySoundByName("Engines", 1.0f, glm::vec3(13.5f, 28.8f, 0));
		PlaySoundByName("Engines", 1.0f, glm::vec3(-13.5f, 28.8f, 0));
		PlaySoundByName(track, 0.5f);
	}
	else
	{
		PlaySoundByName(track, 1.0f);
	}
}

AudioManager::~AudioManager()
{
	studioSystem->release();
}

void AudioManager::Update(float deltaTime) {

	if (GetGameObject()->GetScene()->MainCamera->GetGameObject()->GetParent() != NULL)
	{
		GameObject::Sptr player = GetGameObject()->GetScene()->MainCamera->GetGameObject()->GetParent();

		if (player->Has<SimpleCameraControl>())
		{
			glm::quat dir = player->Get<SimpleCameraControl>()->currentRot;
			FMOD_3D_ATTRIBUTES listenerAttributes;
			listenerAttributes.position = GlmVectorToFmodVector(player->GetPosition());
			listenerAttributes.velocity = GlmVectorToFmodVector(player->Get<Gameplay::Physics::RigidBody>()->GetLinearVelocity());
			listenerAttributes.forward = GlmVectorToFmodVector(dir * glm::vec4(0.0f, 0.0f, -1.0f, 1.0f));
			listenerAttributes.up = GlmVectorToFmodVector(dir * glm::vec4(0.0f, -1.0f, 0.0f, 1.0f));

			studioSystem->setListenerAttributes(0, &listenerAttributes);
		}

		//I apoligize in advance for this nested if statement. Im tired.
		if (Application::Get().scenePath.substr(Application::Get().scenePath.find_last_of("/\\") + 1) == "level2.json")
		{
			if (glm::length(glm::vec3(-40.7f, 115.5f, 6.0f) - player->GetPosition()) <= 20.0f)
			{
				if (saferoomEvent == nullptr)
				{
					saferoomEvent = PlaySoundByName("SafeRoom", 0.8f);
				}
				else
				{
					float vol;
					saferoomEvent->getVolume(&vol);
					saferoomEvent->setVolume(glm::lerp(vol, 0.8f, deltaTime));
				}
			}
			else
			{
				if (saferoomEvent != nullptr)
				{
					float vol;
					saferoomEvent->getVolume(&vol);

					saferoomEvent->setVolume(glm::lerp(vol, 0.0f, deltaTime));

					if (vol <= 0.01f)
						saferoomEvent = nullptr;
				}
			}
		}

	}

	FMOD::ChannelGroup* masterChannelGroup;
	system->getMasterChannelGroup(&masterChannelGroup);
	masterChannelGroup->setVolume(volume);

	studioSystem->update();
}

void AudioManager::RenderImGui() {
	LABEL_LEFT(ImGui::DragFloat, "Volume", &volume);
	ImGui::Text("Track");
	// Draw a textbox for our track name
	static char nameBuff[256];
	memcpy(nameBuff, track.c_str(), track.size());
	nameBuff[track.size()] = '\0';
	if (ImGui::InputText("", nameBuff, 256)) {
		track = nameBuff;
	}
}

nlohmann::json AudioManager::ToJson() const {
	return {
		{ "volume", volume },
		{ "track", track }
	};
}

AudioManager::Sptr AudioManager::FromJson(const nlohmann::json& blob) {
	AudioManager::Sptr result = std::make_shared<AudioManager>();
	result->volume = JsonGet(blob, "volume", result->volume);
	result->track = JsonGet(blob, "track", result->track);
	return result;
}

void AudioManager::LoadSound(const std::string& soundName, const std::string& filename)
{
	FMOD::Studio::EventDescription* eventDesc;
	studioSystem->getEvent(filename.c_str(), &eventDesc);

	events[soundName] = eventDesc;
}

FMOD::Studio::EventInstance* AudioManager::PlaySoundByName(const std::string& soundName, float vol, glm::vec3 pos, bool lowpass, float pitch)
{
	FMOD::Studio::EventInstance* eventInst;
	events[soundName]->createInstance(&eventInst);

	if (pos != glm::vec3(0.0f))
	{
		FMOD_3D_ATTRIBUTES attributes;
		attributes.position = GlmVectorToFmodVector(pos);
		eventInst->set3DAttributes(&attributes);
	}
	eventInst->setVolume(vol);
	eventInst->setPitch(pitch);

	eventInst->start();

	return eventInst;
}

void AudioManager::PlayFootstepSound(glm::vec3 pos, float vol)
{
	FMOD::Studio::EventInstance* eventInst;
	events["Footstep"]->createInstance(&eventInst);

	eventInst->setVolume(vol);
	eventInst->start();
}

void AudioManager::UnloadSound(const std::string& soundName)
{
	events[soundName]->releaseAllInstances();
}

void AudioManager::StopAllSounds()
{
	FMOD::Studio::Bus* masterBus;
	studioSystem->getBus("bus:/", &masterBus);
	masterBus->stopAllEvents(FMOD_STUDIO_STOP_ALLOWFADEOUT);
}


const FMOD_VECTOR AudioManager::GlmVectorToFmodVector(glm::vec3 vec)
{
	FMOD_VECTOR fVec;
	fVec.x = vec.x;
	fVec.y = vec.y;
	fVec.z = vec.z;
	return fVec;
}
