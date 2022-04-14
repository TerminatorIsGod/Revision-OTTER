#version 430

layout (location = 0) in vec2 inUV;
layout (location = 1) in vec3 inViewDir;

layout (location = 0) out vec3 outColor;

uniform layout(binding = 0) sampler2D s_Image;
uniform layout(binding = 1) sampler2D s_Depth;
uniform layout(binding = 2) sampler2D s_Normals;

uniform float u_Intensity;

#include "../../fragments/frame_uniforms.glsl"

float random( vec2 p )
{
  vec2 K1 = vec2(
    23.14069263277926, // e^pi (Gelfond's constant)
    2.665144142690225 // 2^sqrt(2) (Gelfond–Schneider constant)
  );
return fract( cos( dot(p,K1) ) * 12345.6789 );
}

void main() {

    vec3 color = texture(s_Image, inUV).rgb;
    vec2 uvRandom = inUV;
    uvRandom.y *= random(vec2(uvRandom.y,u_Time));
    color += random(uvRandom) * u_Intensity;
    
    outColor = vec3(color);
}
