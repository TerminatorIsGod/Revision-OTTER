#include "Gameplay/Components/RenderComponent.h"

#include "Utils/ResourceManager/ResourceManager.h"

#include "Gameplay/Material.h"


RenderComponent::RenderComponent(const Gameplay::MeshResource::Sptr& mesh, const Gameplay::Material::Sptr& material) :
	_mesh(mesh), 
	_material(material), 
	_meshBuilderParams(std::vector<MeshBuilderParam>()) 
{ }

RenderComponent::RenderComponent() : 
	_mesh(nullptr), 
	_material(nullptr), 
	_meshBuilderParams(std::vector<MeshBuilderParam>())
{ }

void RenderComponent::SetMesh(const Gameplay::MeshResource::Sptr& mesh) {
	_mesh = mesh;
}

const Gameplay::MeshResource::Sptr& RenderComponent::GetMeshResource() const {
	return _mesh;
}

const VertexArrayObject::Sptr& RenderComponent::GetMesh() const {
	return _mesh ? _mesh->Mesh : nullptr;
}

void RenderComponent::SetMaterial(const Gameplay::Material::Sptr& mat) {
	_material = mat;
}

const Gameplay::Material::Sptr& RenderComponent::GetMaterial() const {
	return _material;
}

nlohmann::json RenderComponent::ToJson() const {
	nlohmann::json result;
	result["mesh"] = _mesh ? _mesh->GetGUID().str() : "null";
	result["material"] = _material ? _material->GetGUID().str() : "null";
	return result;
}

RenderComponent::Sptr RenderComponent::FromJson(const nlohmann::json& data) {
	RenderComponent::Sptr result = std::make_shared<RenderComponent>();
	result->_mesh = ResourceManager::Get<Gameplay::MeshResource>(Guid(data["mesh"].get<std::string>()));
	result->_material = ResourceManager::Get<Gameplay::Material>(Guid(data["material"].get<std::string>()));

	return result;
}

void RenderComponent::RenderImGui() {
	ImGui::Text("Indexed:   %s", GetMesh() != nullptr ? (_mesh->Mesh->GetIndexBuffer() != nullptr ? "true" : "false") : "N/A");
	ImGui::Text("Triangles: %d", GetMesh() != nullptr ? (_mesh->Mesh->GetElementCount() / 3) : 0);
	ImGui::Text("Source:    %s", (_mesh == nullptr || _mesh->Filename.empty()) ? "Generated" : _mesh->Filename.c_str());
	ImGui::Separator();
	ImGui::Text("Material:  %s", _material != nullptr ? _material->Name.c_str() : "NULL");
	/*char setMat[25];
	ImGui::InputText("Set Material File", setMat, 25);
	if (ImGui::Button("Set Material")) {
		
		Shader::Sptr basicShader = ResourceManager::CreateAsset<Shader>(std::unordered_map<ShaderPartType, std::string>{
			{ ShaderPartType::Vertex, "shaders/vertex_shader.glsl" },
			{ ShaderPartType::Fragment, "shaders/frag_blinn_phong_textured.glsl" }
		});

		Texture2D::Sptr    textor = ResourceManager::CreateAsset<Texture2D>(setMat);

		Gameplay::Material::Sptr boxMaterial = ResourceManager::CreateAsset<Gameplay::Material>();
		{
			boxMaterial->Name = setMat;
			boxMaterial->MatShader = basicShader;
			boxMaterial->Texture = textor;
			boxMaterial->Shininess = 0.1f;
		}

		
	}*/
}
