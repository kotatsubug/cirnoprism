#pragma once

#include <glew.h> // Must be BEFORE GLFW!
#include <glfw3.h>

#include <glm.hpp>
#include <vec2.hpp>
#include <vec3.hpp>
#include <vec4.hpp>
#include <mat4x4.hpp>
#include <gtc\type_ptr.hpp>

#include "renderer/shader.hh"

class Material
{
private:
	glm::vec3 _ambient;
	glm::vec3 _diffuse;
	glm::vec3 _specular;
	GLint _diffuseTex;
	GLint _specularTex;
	GLint _emissiveTex;
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
		_ambient = ambient;
		_diffuse = diffuse;
		_specular = specular;
		_diffuseTex = diffuseTex;
		_specularTex = specularTex;
		_emissiveTex = emissiveTex;
	}

	~Material()
	{

	}

	// Functions
	void SendToShader(Shader& program)
	{
		program.SetVec3f(_ambient, "material.ambient");
		program.SetVec3f(_diffuse, "material.diffuse");
		program.SetVec3f(_specular, "material.specular");
		program.Set1i(_diffuseTex, "material.diffuseTex");
		program.Set1i(_specularTex, "material.specularTex");
		program.Set1i(_emissiveTex, "material.emissiveTex");
	}
};