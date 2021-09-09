#pragma once

#include <iostream>
#include <fstream>
#include <string>

#include <glew.h> // Must be BEFORE GLFW!
#include <glfw3.h>

#include <glm.hpp> // TODO: don't need a lot of these
#include <vec2.hpp>
#include <vec3.hpp>
#include <vec4.hpp>
#include <mat4x4.hpp>
#include <gtc\type_ptr.hpp>

#include "shader.hh"

#define _SHADOW_MAP_ONLY 1

struct FramebufferType
{
	int width;
	int height;
	int samples = 1;
};

class Framebuffer
{
protected:
	GLuint id;
	FramebufferType type;
	GLuint colorAttachmentID;
	GLuint depthMapID;

	glm::mat4 lightProjection;
public:
	Framebuffer(const FramebufferType& type, glm::vec3 lightPos);
	~Framebuffer();

	virtual void Initialize();
	void Bind();
	void UnBind();
	
	void SendToShader(Shader& shader)
	{
		shader.SetMat4fv(this->lightProjection, "lightProjection");
		shader.Set1i(this->id, "shadowMap");
	}

	inline const FramebufferType& GetFramebufferType() const { return this->type; }
	inline int GetColorAttachmentID() const { return this->colorAttachmentID; }
	inline int GetDepthMapID() const { return this->depthMapID; }
	inline glm::mat4 GetLightProjectionMatrix() const { return this->lightProjection; }
};
