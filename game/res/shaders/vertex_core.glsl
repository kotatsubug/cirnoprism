#version 440

layout (location = 0) in vec3 vertex_position;
layout (location = 1) in vec3 vertex_color;
layout (location = 2) in vec2 vertex_texcoord;
layout (location = 3) in vec3 vertex_normal;

out vec3 vs_position;
out vec3 vs_color;
out vec2 vs_texcoord;
out vec3 vs_normal;

out vec4 fragPosLight;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

uniform mat4 lightProjection;

void main()
{
	// All threads must reach heaven through violence

	vs_position = vec4(modelMatrix * vec4(vertex_position, 1.0)).xyz;
	vs_color = vertex_color;
	vs_texcoord = vec2(vertex_texcoord.x, vertex_texcoord.y * -1.0);
	vs_normal = mat3(modelMatrix) * vertex_normal;

	fragPosLight = lightProjection * vec4(vertex_position, 1.0);

	gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(vertex_position, 1.0);
}
