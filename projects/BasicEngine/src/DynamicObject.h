#pragma once

#include <bullet/btBulletDynamicsCommon.h>
#include <bullet/btBulletCollisionCommon.h>
#include <VertexArrayObject.h>
#include <Shader.h>
#include <GLM/gtc/matrix_transform.hpp>

class DynamicObject
{


public:

	DynamicObject(
		glm::vec3 objectPos,
		glm::vec3 size,
		btScalar mass,
		btDiscreteDynamicsWorld* world, VertexArrayObject::Sptr renderObject, std::string nickname);

	void ApplyForce(glm::vec3 forceDir);
	void SetVelocity(glm::vec3 velocityDir);
	void Draw(const std::string& name, const glm::highp_mat4& value);
	bool LoadShaderFiles(const char* pathVertex, const char* pathFragment);
	glm::vec3 GetPosition();

	bool IsObjectOnGround() {
		if (abs(m_rigidBoby->getLinearVelocity().getY()) > 0.2) {
			return false;
		}
		return true;
	}

	void CleanUp();

	btTransform getTransform();

private:

	std::string m_name;

	btTransform m_transform;

	// physics
	btCollisionShape* m_collisionShape = nullptr;
	btScalar m_Mass = btScalar(1.f);
	btVector3 m_LocalInertia = btVector3(0, 0, 0);
	btDefaultMotionState* m_motionState = nullptr;
	btRigidBody* m_rigidBoby = nullptr;
	VertexArrayObject::Sptr m_renderedObject = nullptr;
	Shader* m_shader = nullptr;

};

