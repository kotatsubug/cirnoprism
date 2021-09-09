#version 440

out vec4 FragColor;

in vec2 texCoords;

uniform sampler2D screenTexture;

void main()
{
	FragColor = vec4(vec3(1.0 - texture(screenTexture, texCoords).r, 1.0 - texture(screenTexture, texCoords).g, 1.0 - texture(screenTexture, texCoords).b), 1);
}
