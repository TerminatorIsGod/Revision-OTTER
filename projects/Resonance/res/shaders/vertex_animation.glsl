#version 410

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec2 inUV;

layout(location = 7) in vec3 inPos2;
layout(location = 8) in vec3 inCol2;
layout(location = 9) in vec3 inNorm2;
layout(location = 10) in vec2 inUV2;

layout(location = 0) out vec3 outWorldPos;
layout(location = 1) out vec3 outColor;
layout(location = 2) out vec3 outNormal;
layout(location = 3) out vec2 outUV;

layout(location = 7) out vec3 outPos2;
layout(location = 8) out vec3 outCol2;
layout(location = 9) out vec3 outNorm2;
layout(location = 10) out vec2 outUV2;

// Complete MVP
uniform mat4 u_ModelViewProjection;
// Just the model transform, we'll do worldspace lighting
uniform mat4 u_Model;
// Normal Matrix for transforming normals
uniform mat3 u_NormalMatrix;

uniform float delta = 0;
uniform float randomx = 0;
uniform float randomy = 0;
uniform float randomz = 0;

uniform int segment;

vec3 randomPos;

//previous segment will have 1 - delta weight

void main() {

	randomPos = vec3(randomx, randomy, randomz);

	vec3 pos = mix(inPos2, randomPos, delta);

	gl_Position = u_ModelViewProjection * vec4(pos, 1.0);

	// Lecture 5
	// Pass vertex pos in world space to frag shader
	outWorldPos = (u_Model * vec4(pos, 1.0)).xyz;

	// Normals
	outNormal = u_NormalMatrix * inNorm2;

	// Pass our UV coords to the fragment shader
	outUV = inUV2;

	///////////
	outColor = inCol2;

}

