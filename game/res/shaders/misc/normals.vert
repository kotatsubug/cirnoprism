#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 3) in vec3 aNormal;

out VS_OUT {
	vec3 normal;
} vs_out;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;

void main()
{
	gl_Position = viewMatrix * modelMatrix * vec4(aPos, 1.0); 
	mat3 normalMatrix = mat3(transpose(inverse(viewMatrix * modelMatrix)));
	vs_out.normal = normalize(vec3(vec4(normalMatrix * aNormal, 0.0)));
}