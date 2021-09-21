#pragma once

#include "libs.hh"

class Light
{
protected:
	glm::vec3 _color;
	float _intensity;
public:
	Light(glm::vec3 color, float intensity)
	{
		_color = color;
		_intensity = intensity;
	}

	~Light()
	{
	}

	virtual void SendToShader(Shader& program) = 0;
};

class PointLight : Light
{
protected:
	glm::vec3 _position;
	float _constant, _linear, _quadratic;
public:
	PointLight(glm::vec3 position, glm::vec3 color = glm::vec3(1.0f), float intensity = 1.0f,
		float constant = 1.0f, float linear = 0.045f, float quadratic = 0.0075f)
		: Light(color, intensity)
	{
		_position = position;
		_constant = constant;
		_linear = linear;
		_quadratic = quadratic;
	}

	~PointLight()
	{

	}

	void SendToShader(Shader& program)
	{
		program.SetVec3f(_position, "pointLight.position");
		program.SetVec3f(_color, "pointLight.color");
		program.Set1f(_intensity, "pointLight.intensity");
		program.Set1f(_constant, "pointLight.constant");
		program.Set1f(_linear, "pointLight.linear");
		program.Set1f(_quadratic, "pointLight.quadratic");
	}

	void SetPosition(const glm::vec3 val)
	{
		_position = val;
	}
};