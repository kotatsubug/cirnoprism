#pragma once

#include <glew.h> // Must be BEFORE GLFW!
#include <glfw3.h>

#include <glm.hpp>
#include <vec2.hpp>
#include <vec3.hpp>
#include <vec4.hpp>
#include <mat4x4.hpp>
#include <gtc\type_ptr.hpp>

#include "shader.hh"

class Material
{
private:
	glm::vec3 ambient;
	glm::vec3 diffuse;
	glm::vec3 specular;
	GLint diffuseTex;
	GLint specularTex;
	GLint emissiveTex;
public:
	Material(
		glm::vec3 ambient,
		glm::vec3 diffuse,
		glm::vec3 specular,
		GLint diffuseTex,
		GLint specularTex,
		GLint emissiveTex
	)
	{
		this->ambient = ambient;
		this->diffuse = diffuse;
		this->specular = specular;
		this->diffuseTex = diffuseTex;
		this->specularTex = specularTex;
		this->emissiveTex = emissiveTex;
	}

	~Material()
	{

	}

	// Functions
	void SendToShader(Shader& program)
	{
		program.SetVec3f(this->ambient, "material.ambient");
		program.SetVec3f(this->diffuse, "material.diffuse");
		program.SetVec3f(this->specular, "material.specular");
		program.Set1i(this->diffuseTex, "material.diffuseTex");
		program.Set1i(this->specularTex, "material.specularTex");
		program.Set1i(this->emissiveTex, "material.emissiveTex");
	}
};