#version 440
layout (location = 0) in vec3 aPos;

uniform mat4 lightProjection;
uniform mat4 modelMatrix;

void main()
{
	gl_Position = lightProjection * modelMatrix * vec4(aPos, 1.0);
}
