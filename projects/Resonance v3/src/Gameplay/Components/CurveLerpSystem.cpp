#include "Gameplay/Components/CurveLerpSystem.h"
#include <GLFW/glfw3.h>
#include "Gameplay/GameObject.h"
#include "Gameplay/Scene.h"
#include "Utils/ImGuiHelper.h"

void CurveLerpSystem::Awake()
{

}

void CurveLerpSystem::RenderImGui() {
	LABEL_LEFT(ImGui::DragFloat, "Curve time", &tLength);

}

nlohmann::json CurveLerpSystem::ToJson() const {
	return {
		{ "cocklength", tLength}
	};
}

CurveLerpSystem::CurveLerpSystem() :
	IComponent(),
	tLength(1)
{ }

CurveLerpSystem::~CurveLerpSystem() = default;

CurveLerpSystem::Sptr CurveLerpSystem::FromJson(const nlohmann::json & blob) {
	CurveLerpSystem::Sptr result = std::make_shared<CurveLerpSystem>();

	result->tLength = blob["cocklength"];
	return result;
}

void CurveLerpSystem::Update(float deltaTime) {

	//CATMULL

	t += deltaTime;

	if (t >= tLength) {
		t = 0;
		segment++;
		if (segment >= points.size())
			segment = 0;
	}

	float timer = t / tLength;

	int p0_index, p1_index, p2_index, p3_index;
	glm::vec3 p0, p1, p2, p3;

	p1_index = segment;
	p2_index = (segment + 1 > points.size() - 1) ? 0 : segment + 1;
	p3_index = (p2_index + 1 > points.size() - 1) ? 0 : p2_index + 1;
	p0_index = (p1_index == 0) ? points.size() - 1 : p1_index - 1;

	p0 = points[p0_index];
	p1 = points[p1_index];
	p2 = points[p2_index];
	p3 = points[p3_index];

	float newx = 0;
	float newy = 0;
	float newz = 0;

	newx = 0.5 * ((2 * p1.x) + (-p0.x + p2.x) * timer + (2 * p0.x - 5 * p1.x + 4 * p2.x - p3.x) * pow(timer, 2) + (-p0.x + 3 * p1.x - 3 * p2.x + p3.x) * pow(timer, 3));
	newy = 0.5 * ((2 * p1.y) + (-p0.y + p2.y) * timer + (2 * p0.y - 5 * p1.y + 4 * p2.y - p3.y) * pow(timer, 2) + (-p0.y + 3 * p1.y - 3 * p2.y + p3.y) * pow(timer, 3));
	newz = 0.5 * ((2 * p1.z) + (-p0.z + p2.z) * timer + (2 * p0.z - 5 * p1.z + 4 * p2.z - p3.z) * pow(timer, 2) + (-p0.z + 3 * p1.z - 3 * p2.z + p3.z) * pow(timer, 3));

	GetGameObject()->SetPostion(glm::vec3(newx, newy, newz));

	//GetGameObject()->LookAt(glm::vec3(newx, newy, newz));

}
