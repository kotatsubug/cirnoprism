#pragma once

#include "../libs.hh"

class Light
{
protected:
	glm::vec3 color;
	float intensity;
	
public:
	Light(glm::vec3 color, float intensity)
	{
		this->color = color;
		this->intensity = intensity;
	}

	~Light()
	{

	}

	virtual void SendToShader(Shader& program) = 0;
};

class PointLight : Light
{
protected:
	glm::vec3 position;
	float constant, linear, quadratic;
public:
	PointLight(glm::vec3 position, glm::vec3 color = glm::vec3(1.0f), float intensity = 1.0f,
		float constant = 1.0f, float linear = 0.045f, float quadratic = 0.0075f)
		: Light(color, intensity)
	{
		this->position = position;
		this->constant = constant;
		this->linear = linear;
		this->quadratic = quadratic;
	}

	~PointLight()
	{

	}

	void SendToShader(Shader& program)
	{
		program.SetVec3f(this->position, "pointLight.position");
		program.SetVec3f(this->color, "pointLight.color");
		program.Set1f(this->intensity, "pointLight.intensity");
		program.Set1f(this->constant, "pointLight.constant");
		program.Set1f(this->linear, "pointLight.linear");
		program.Set1f(this->quadratic, "pointLight.quadratic");
	}

	void SetPosition(const glm::vec3 val)
	{
		this->position = val;
	}
};