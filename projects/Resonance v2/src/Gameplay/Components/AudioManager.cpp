#include "Gameplay/Components/AudioManager.h"
#include "Utils/ImGuiHelper.h"
#include "Utils/JsonGlmHelpers.h"

void AudioManager::Awake() {
	FMOD::System_Create(&system);
	system->init(32, FMOD_INIT_NORMAL, nullptr);

	LoadSound("L1_Ambiance", "Audio/Music/Infested_Engines.wav", true, true);

	PlaySoundByName("L1_Ambiance");
}

AudioManager::~AudioManager()
{
	system->release();
}

void AudioManager::Update(float deltaTime) {
	GetGameObject()->GetScene();
	system->update();
}

void AudioManager::RenderImGui() {
	LABEL_LEFT(ImGui::DragFloat, "Ambiance Volume", &volume);
}

nlohmann::json AudioManager::ToJson() const {
	return {
		{ "volume", volume }
	};
}

AudioManager::Sptr AudioManager::FromJson(const nlohmann::json& blob) {
	AudioManager::Sptr result = std::make_shared<AudioManager>();
	result->volume = JsonGet(blob, "volume", result->volume);
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

void AudioManager::PlaySoundByName(const std::string& soundName)
{
	system->playSound(sounds[soundName], nullptr, false, nullptr);
}

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
