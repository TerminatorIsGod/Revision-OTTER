#include "FilmGrain.h"
#include "Utils/ResourceManager/ResourceManager.h"
#include "Utils/JsonGlmHelpers.h"
#include "Utils/ImGuiHelper.h"

FilmGrain::FilmGrain() :
	PostProcessingLayer::Effect(),
	_shader(nullptr),
	_intensity(0.05)
{
	Name = "Film Grain Effect";
	_format = RenderTargetType::ColorRgb8;

	_shader = ResourceManager::CreateAsset<ShaderProgram>(std::unordered_map<ShaderPartType, std::string>{
		{ ShaderPartType::Vertex, "shaders/vertex_shaders/fullscreen_quad.glsl" },
		{ ShaderPartType::Fragment, "shaders/fragment_shaders/post_effects/film_grain.glsl" }
	});
}

FilmGrain::~FilmGrain() = default;

void FilmGrain::Apply(const Framebuffer::Sptr & gBuffer)
{
	_shader->Bind();
	_shader->SetUniform("u_Intensity", _intensity);
	gBuffer->BindAttachment(RenderTargetAttachment::Depth, 1);
	gBuffer->BindAttachment(RenderTargetAttachment::Color1, 2); // The normal buffer
}

void FilmGrain::RenderImGui()
{
	LABEL_LEFT(ImGui::SliderFloat, "Intensity", &_intensity, 0.0f, 1.0f);
}

FilmGrain::Sptr FilmGrain::FromJson(const nlohmann::json & data)
{
	FilmGrain::Sptr result = std::make_shared<FilmGrain>();

	result->_intensity = JsonGet(data, "intensity", result->_intensity);

	return result;
}

nlohmann::json FilmGrain::ToJson() const
{
	return {
		{ "intensity", _intensity },
	};
}