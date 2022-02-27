#include "Gameplay/Components/AudioManager.h"
#include "Utils/ImGuiHelper.h"
#include "Utils/JsonGlmHelpers.h"
#include "Gameplay/Components/SimpleCameraControl.h"

void AudioManager::Awake() {
	GetGameObject()->GetScene()->audioManager = GetGameObject();

	FMOD::System_Create(&system);
	system->init(32, FMOD_INIT_NORMAL, nullptr);

	LoadSound("L1_Ambiance", "Audio/Music/Infested_Engines.wav", false, true);
	LoadSound("Title", "Audio/Music/Resonance.wav", false, true);
	LoadSound("L2_Ambiance", "Audio/Music/Dead Quarters.wav", false, true);
	LoadSound("Transition", "Audio/Sounds/loadingTransition.wav", false, false);
	LoadSound("Footstep", "Audio/Sounds/footstepTest4.wav", false, false);
	LoadSound("LeaflingPatrol", "Audio/Sounds/Leaflings_Patrol2.wav", true, true);
	LoadSound("LeaflingDistracted", "Audio/Sounds/Leaflings_Distracted.wav", true, true);
	LoadSound("LeaflingAgro", "Audio/Sounds/Leaflings_Agro.wav", true, true);
	LoadSound("Engines", "Audio/Sounds/engineWhirring.wav", true, true);
	LoadSound("OxygenRefill", "Audio/Sounds/replenishOxygen.wav", false, true);
	LoadSound("OutOfBreath", "Audio/Sounds/outOfBreath.wav", false, true);
	LoadSound("HoldingBreath", "Audio/Sounds/holdingBreath.wav", false, false);
	LoadSound("BreathOut", "Audio/Sounds/breathOut.wav", false, false);
	LoadSound("ValveTwist", "Audio/Sounds/valveTwist2.wav", true, false);
	LoadSound("VendingMachine", "Audio/Sounds/vendingMachine.wav", true, false);
	LoadSound("IdleIn", "Audio/Sounds/idleIn.wav", false, false);
	LoadSound("IdleOut", "Audio/Sounds/idleOut.wav", false, false);
	LoadSound("StopReplenish", "Audio/Sounds/stopReplenishOxygen.wav", false, false);



	if (track == "L1_Ambiance")
	{
		FMOD::Channel* tempChannel;
		system->playSound(sounds[track], nullptr, false, &tempChannel);
		tempChannel->set3DAttributes(&GlmVectorToFmodVector(glm::vec3(0, 50, -7)), 0);
		tempChannel->setVolume(1.0f);

		FMOD::Channel* tempChannel2;
		system->playSound(sounds["Engines"], nullptr, false, &tempChannel2);
		tempChannel2->set3DAttributes(&GlmVectorToFmodVector(glm::vec3(13.5f, 28.8f, 0)), 0);
		tempChannel2->setVolume(3.0f);

		FMOD::Channel* tempChannel3;
		system->playSound(sounds["Engines"], nullptr, false, &tempChannel3);
		tempChannel3->set3DAttributes(&GlmVectorToFmodVector(glm::vec3(-13.5f, 28.8f, 0)), 0);
		tempChannel3->setVolume(3.0f);
	}
	else
	{
		PlaySoundByName(track);
	}
}

AudioManager::~AudioManager()
{
	system->release();
}

void AudioManager::Update(float deltaTime) {

	GameObject* player = GetGameObject()->GetScene()->MainCamera->GetGameObject();
	if (player->Has<SimpleCameraControl>())
	{
		glm::quat dir = player->Get<SimpleCameraControl>()->currentRot;
		system->set3DListenerAttributes(0, &GlmVectorToFmodVector(player->GetPosition()), 0, &GlmVectorToFmodVector(dir * glm::vec4(0.0f, 0.0f, -1.0f, 1.0f)), &GlmVectorToFmodVector(dir * glm::vec4(0.0f, -1.0f, 0.0f, 1.0f)));
	}
	system->update();

	FMOD::ChannelGroup* masterChannelGroup;
	system->getMasterChannelGroup(&masterChannelGroup);
	masterChannelGroup->setVolume(volume);
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

void AudioManager::LoadSound(const std::string& soundName, const std::string& filename, bool b3d, bool bLooping, bool bStream)
{
	//Check if already loaded
	auto foundElement = sounds.find(soundName);
	if (foundElement != sounds.end())
	{
		//has already been loaded.
		return;
	}

	FMOD_MODE mode = FMOD_DEFAULT;
	mode |= (b3d) ? FMOD_3D : FMOD_2D;
	mode |= (bLooping) ? FMOD_LOOP_NORMAL : FMOD_LOOP_OFF;
	mode |= (bStream) ? FMOD_CREATESTREAM : FMOD_CREATECOMPRESSEDSAMPLE;

	FMOD::Sound* loadedSound;
	system->createSound(filename.c_str(), mode, nullptr, &loadedSound);
	if (loadedSound != nullptr)
		sounds[soundName] = loadedSound;
}

FMOD::Channel* AudioManager::PlaySoundByName(const std::string& soundName, float vol, glm::vec3 pos)
{
	FMOD::Channel* newChannel;
	system->playSound(sounds[soundName], nullptr, false, &newChannel);
	newChannel->setVolume(vol);
	newChannel->set3DAttributes(&GlmVectorToFmodVector(pos), 0);

	return newChannel;
}

void AudioManager::PlayFootstepSound(glm::vec3 pos, float vol)
{
	//make if statement that only plays sound if < x amount of channels are being used.
	system->playSound(sounds["Footstep"], nullptr, false, &footstepChannel);

	float rVolume = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 0.3f));
	footstepChannel->setVolume(vol + rVolume);

	float rPitch = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 0.4f));
	footstepChannel->setPitch(rPitch + 0.7f);

	footstepChannel->set3DAttributes(&GlmVectorToFmodVector(pos), 0);
}

//void AudioManager::PauseSoundByName(const std::string& soundName)
//{
//	//system->playSound(sounds[soundName], nullptr, true, nullptr);
//	
//}

void AudioManager::UnloadSound(const std::string& soundName)
{
	//Check if already loaded
	auto foundElement = sounds.find(soundName);
	if (foundElement != sounds.end())
	{
		foundElement->second->release();
		sounds.erase(foundElement);
	}
}

const FMOD_VECTOR AudioManager::GlmVectorToFmodVector(glm::vec3 vec)
{
	FMOD_VECTOR fVec;
	fVec.x = vec.x;
	fVec.y = vec.y;
	fVec.z = vec.z;
	return fVec;
}
