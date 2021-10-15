#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include <glew.h> // Must be BEFORE GLFW!
#include <glfw3.h>

#include <glm.hpp> // TODO: don't need a lot of these
#include <vec2.hpp>
#include <vec3.hpp>
#include <vec4.hpp>
#include <mat4x4.hpp>
#include <gtc\type_ptr.hpp>

#include "common.hh"

class Shader
{
private:
	GLuint _id; // Program ID, created in main. Needed to direct calls
	const int _glVersionMajor; // OpenGL versions
	const int _glVersionMinor;

	std::string _LoadShaderFile(const std::string& filename);
	GLuint _LoadShader(GLenum shaderType, const std::string& filename);
	void _LinkProgram(GLuint vertexShader, GLuint geometryShader, GLuint fragmentShader);
public:
	Shader(
		const int glVersionMajor,
		const int glVersionMinor,
		const std::string& vertexFile,
		const std::string& fragmentFile,
		const std::string& geometryFile = ""
	);
	~Shader();
	void Use();
	void UnUse();
	void Set1i(const int val, const std::string& name);
	void Set1f(const float val, const std::string& name);
	void SetVec2f(const glm::fvec2& val, const std::string& name);
	void SetVec3f(const glm::fvec3& val, const std::string& name);
	void SetVec4f(const glm::fvec4& val, const std::string& name);
	void SetMat3fv(const glm::mat3& matrix, const std::string& name, GLboolean transpose = GL_FALSE);
	void SetMat4fv(const glm::mat4& matrix, const std::string& name, GLboolean transpose = GL_FALSE);
	void SetArrMat4fv(const std::vector<glm::mat4>& matrices, const std::string& name, GLboolean transpose = GL_FALSE);

	/// This should only be used for debugging purposes.
	inline GLuint GetID() { return _id; }
};