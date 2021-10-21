#include "Player.h"


Player::Player(
	glm::vec3 cameraPos,
	glm::vec3 playerPos,
	glm::vec3 size,
	Camera::Sptr camera,
	btDiscreteDynamicsWorld* world, VertexArrayObject::Sptr renderObject, std::string nickname) {

	m_renderedObject = renderObject;

	m_camera = camera;
	m_camera->SetFovDegrees(90.0f);
	m_cameraPos = cameraPos;

	m_name = nickname;

	m_collisionShape = new btCapsuleShape(btScalar(.5), btScalar(0.1));
	btTransform startTransform;
	startTransform.setIdentity();
	m_collisionShape->calculateLocalInertia(m_Mass, m_LocalInertia);
	startTransform.setOrigin(btVector3(m_cameraPos.x, m_cameraPos.y, m_cameraPos.z));

	m_motionState = new btDefaultMotionState(startTransform);
	btRigidBody::btRigidBodyConstructionInfo rbInfo(m_Mass, m_motionState,
		m_collisionShape, m_LocalInertia);

	m_rigidBoby = new btRigidBody(rbInfo);
	m_rigidBoby->setAngularFactor(btVector3(0, 1, 0));
	m_rigidBoby->setDamping(.95, 0.9);

	//m_rigidBoby->setCollisionFlags(m_rigidBoby->getCollisionFlags() | btCollisionObject::CF_STATIC_OBJECT);
	m_rigidBoby->setActivationState(DISABLE_DEACTIVATION);
	m_rigidBoby->activate(true);

	m_shader = new Shader();

	world->addRigidBody(m_rigidBoby, 1, 1);
}

void Player::Update(glm::vec3 cameraDir, glm::vec3 cameraUp) {

	btTransform trans;
	if (m_rigidBoby && m_rigidBoby->getMotionState())
	{
		m_rigidBoby->getMotionState()->getWorldTransform(trans);
	}
	glm::vec3 pos = glm::vec3(
		trans.getOrigin().getX(),
		trans.getOrigin().getY(),
		trans.getOrigin().getZ());

	m_camera->SetPosition(pos);
	m_camera->SetForward(cameraDir);
	m_camera->SetUp(cameraUp);
}

void Player::ApplyForce(glm::vec3 forceDir)
{
	m_rigidBoby->activate(true);
	m_rigidBoby->applyCentralForce(btVector3(forceDir.x, forceDir.y, forceDir.z));
	//m_rigidBoby->applyForce(btVector3(forceDir.x, forceDir.y, forceDir.z), btVector3(m_cameraPos.x, m_cameraPos.y, m_cameraPos.z));
}

void Player::SetVelocity(glm::vec3 velocityDir)
{
	if (IsPlayerOnGround()) {
		m_rigidBoby->activate(true);
		m_rigidBoby->setLinearVelocity(btVector3(velocityDir.x, velocityDir.y, velocityDir.z));
	}

}

void Player::CleanUp()
{
	delete m_collisionShape;
	delete m_motionState;
	delete m_rigidBoby;
}

bool Player::LoadShaderFiles(const char* pathVertex, const char* pathFragment) {
	m_shader->LoadShaderPartFromFile(pathVertex, ShaderPartType::Vertex);
	m_shader->LoadShaderPartFromFile(pathFragment, ShaderPartType::Fragment);
	return m_shader->Link();
}

void Player::Draw(const std::string& name, const glm::highp_mat4& value) {
	glm::mat4 transform = glm::translate(glm::mat4(1.0f), GetPosition());
	m_shader->Bind();
	m_shader->SetUniformMatrix(name, value * transform);
	m_renderedObject->Draw();
}

glm::vec3 Player::GetPosition() {
	btTransform trans;
	if (m_rigidBoby && m_rigidBoby->getMotionState())
	{
		m_rigidBoby->getMotionState()->getWorldTransform(trans);
	}
	glm::vec3 pos = glm::vec3(
		trans.getOrigin().getX(),
		trans.getOrigin().getY(),
		trans.getOrigin().getZ());

	return pos;
}