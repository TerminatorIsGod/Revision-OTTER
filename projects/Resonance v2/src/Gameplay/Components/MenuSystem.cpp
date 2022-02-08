#include "Gameplay/Components/MenuSystem.h"
#include <GLFW/glfw3.h>
#include "Utils/ImGuiHelper.h"

namespace Gameplay {
	void MenuSystem::Awake()
	{
		/*Shader::Sptr unlitShader = ResourceManager::CreateAsset<Shader>(std::unordered_map<ShaderPartType, std::string>{
			{ ShaderPartType::Vertex, "shaders/vertex_shaders/basic.glsl" },
			{ ShaderPartType::Fragment, "shaders/fragment_shaders/frag_textured_unlit.glsl" }
		});

		Material::Sptr mainmenuMaterial = ResourceManager::CreateAsset<Material>(unlitShader);
		{
			Texture2D::Sptr    mainmenuTex = ResourceManager::CreateAsset<Texture2D>("textures/Resonace_title_screen.png");

			mainmenuMaterial->Name = "MainMenu";
			mainmenuMaterial->Set("u_Material.Diffuse", mainmenuTex);
			mainmenuMaterial->Set("u_Material.Shininess", 0.0f);
		}

		Material::Sptr pausemenuMaterial = ResourceManager::CreateAsset<Material>(unlitShader);
		{
			Texture2D::Sptr    pausemenuTex = ResourceManager::CreateAsset<Texture2D>("textures/Pause_screen.png");

			pausemenuMaterial->Name = "PauseMenu";
			pausemenuMaterial->Set("u_Material.Diffuse", pausemenuTex);
			pausemenuMaterial->Set("u_Material.Shininess", 0.0f);
		}*/

		MeshResource::Sptr planeMesh = ResourceManager::CreateAsset<MeshResource>();
		planeMesh->AddParam(MeshBuilderParam::CreatePlane(glm::vec4(0.0f), glm::vec4(0.0f, 0.0f, 1.0f, 0.0f), glm::vec4(1.0f, 0.0f, 0.0f, 0.0f), glm::vec2(1.0f)));
		planeMesh->GenerateMesh();

		_mainScene = GetGameObject()->GetScene();

		GetGameObject()->Get<RenderComponent>()->SetMesh(planeMesh);

		if (GetGameObject()->Name == "MenuPlane") {
			if (GetGameObject()->GetScene()->FindObjectByName("Menu Camera")) {
				_menucamera = GetGameObject()->GetScene()->FindObjectByName("Menu Camera")->Get<Camera>();
			}
			else {
				createCamera();
			}
			//GetGameObject()->Get<RenderComponent>()->SetMaterial(mainmenuMaterial);
		}
		else {
			//GetGameObject()->Get<RenderComponent>()->SetMaterial(pausemenuMaterial);
		}

	}

	void MenuSystem::createCamera() {
		GameObject::Sptr camera = _mainScene->CreateGameObject("Menu Camera");
		{
			camera->SetPostion(glm::vec3(0.0f, 0.0f, -100.0f));
			camera->LookAt(glm::vec3(0.0f));
			camera->SetRotation(glm::vec3(0.0f, -90.0f, 0.0f));

			Camera::Sptr cam = camera->Add<Camera>();
		}

		_menucamera = camera->Get<Camera>();
	}

	void MenuSystem::RenderImGui() {
		//LABEL_LEFT(ImGui::DragInt, "Keys", &_keys, 1.0f);
	}

	nlohmann::json MenuSystem::ToJson() const {
		return {
			//{ "keys", _keys }
		};
	}

	MenuSystem::MenuSystem() :
		IComponent()//,
		//_keys(0)
	{
	}

	MenuSystem::~MenuSystem() = default;

	MenuSystem::Sptr MenuSystem::FromJson(const nlohmann::json& blob) {
		MenuSystem::Sptr result = std::make_shared<MenuSystem>();
		//result->_keys = blob["keys"];
		return result;
	}

	void MenuSystem::Update(float deltaTime) {

	}

	Camera::Sptr MenuSystem::getMenuCamera()
	{
		return _menucamera;
	}

	void MenuSystem::setPauseMaterial(Scene::Sptr scene)
	{
		GameObject::Sptr menuPlane = scene->FindObjectByName("MenuPlane");
		RenderComponent::Sptr render = menuPlane->Get<RenderComponent>();
		//render->SetMaterial(_materialPause);
	}

	void MenuSystem::setMenuMaterial(Scene::Sptr scene)
	{
		GameObject::Sptr menuPlane = scene->FindObjectByName("MenuPlane");
		RenderComponent::Sptr render = menuPlane->Get<RenderComponent>();
		//render->SetMaterial(_materialMain);
	}

	void MenuSystem::mainScene(Scene::Sptr scene) {
		//_mainScene = scene;
	}

}



