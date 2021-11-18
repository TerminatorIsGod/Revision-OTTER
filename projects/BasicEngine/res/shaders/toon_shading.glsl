#version 430

layout(location = 0) in vec3 inWorldPos;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec2 inUV;

// We output a single color to the color buffer
layout(location = 0) out vec4 frag_color;

// Represents a collection of attributes that would define a material
// For instance, you can think of this like material settings in 
// Unity
struct Material {
	sampler2D Diffuse;
	float     Shininess;
    int       Steps;
};
// Create a uniform for the material
uniform Material u_Material;

#include "fragments/multiple_point_lights.glsl"

uniform vec3  u_CamPos;


// https://learnopengl.com/Advanced-Lighting/Advanced-Lighting
void main() {
	// Normalize our input normal
	vec3 normal = normalize(inNormal);
	vec3 V = normalize(u_CamPos.xyz - inWorldPos);

	// Use the lighting calculation that we included from our partial file
	vec3 lightAccumulation = CalcAllLightContribution(inWorldPos, normal, u_CamPos.xyz, u_Material.Shininess);

	// Get the albedo from the diffuse / albedo map
	vec4 textureColor = texture(u_Material.Diffuse, inUV);

	float edge = (dot(V, normal) < 0.6) ? 0.0 : 1.0;

	// combine for the final result
	vec3 result = lightAccumulation  * inColor * textureColor.rgb * edge;


    // Simple way to create cel shading effect
    result = round(result * 5) / 5;

	frag_color = vec4(result, textureColor.a);
}