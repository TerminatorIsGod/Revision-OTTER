#pragma once
#include <Camera.h>
#include <bullet/btBulletDynamicsCommon.h>
#include <bullet/btBulletCollisionCommon.h>
#include <VertexArrayObject.h>
#include <Shader.h>
#include <GLM/gtc/matrix_transform.hpp>

class Player
{

public:

	Player(
		glm::vec3 cameraPos,
		glm::vec3 playerPos,
		glm::vec3 size,
		Camera::Sptr camera,
		btDiscreteDynamicsWorld* world, VertexArrayObject::Sptr renderObject, std::string nickname);

	void ApplyForce(glm::vec3 forceDir);
	void SetVelocity(glm::vec3 velocityDir);
	void Draw(const std::string& name, const glm::highp_mat4& value);
	bool LoadShaderFiles(const char *pathVertex, const char *pathFragment);
	void Update(glm::vec3 cameraDir, glm::vec3 cameraUp);
	glm::vec3 GetPosition();

	bool IsPlayerOnGround() {
		if (abs(m_rigidBoby->getLinearVelocity().getY()) > 0.02) {
			return false;
		}
		return true;
	}

	void CleanUp();

private:

	std::string m_name;

	Camera::Sptr m_camera;
	glm::vec3 m_cameraPos = glm::vec3(3.0f, 1.3f, 2);

	// physics
	btCollisionShape* m_collisionShape = nullptr;
	btScalar m_Mass = btScalar(1.f);
	btVector3 m_LocalInertia = btVector3(0, 0, 0);
	btDefaultMotionState* m_motionState = nullptr;
	btRigidBody* m_rigidBoby = nullptr;
	VertexArrayObject::Sptr m_renderedObject = nullptr;
	Shader* m_shader = nullptr;
};

