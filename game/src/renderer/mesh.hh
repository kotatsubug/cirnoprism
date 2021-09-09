#pragma once

#include <iostream>
#include <vector>

#include "vertex.hh"
#include "shader.hh"
#include "texture.hh"
#include "material.hh"
#include "primitives.hh"
#include "../objloader.hh"

class Mesh
{
private:
	Vertex* _vertices; // Vertex array
	uint32_t _numVertices;
	GLuint* _indices; // Index array
	uint32_t _numIndices;

	GLuint _vertexArrayObject;
	GLuint _vertexArrayBuffer;
	GLuint _elementArrayBuffer;

	glm::vec3 _position;
	glm::vec3 _origin;
	glm::vec3 _rotation;
	glm::vec3 _scale;

	glm::mat4 _modelMatrix;

	void _InitMeshBuffers();
	void _UpdateModelMatrix();
	void _UpdateUniforms(Shader* shader);
public:
	Mesh(
		Vertex* vertices, const uint32_t& numVertices,
		GLuint* indices, const uint32_t& numIndices,
		glm::vec3 position = glm::vec3(0.0f),
		glm::vec3 origin = glm::vec3(0.0f),
		glm::vec3 rotation = glm::vec3(0.0f),
		glm::vec3 scale = glm::vec3(1.0f)
	);
	Mesh(
		Primitive* primitive,
		glm::vec3 position = glm::vec3(0.0f),
		glm::vec3 origin = glm::vec3(0.0f),
		glm::vec3 rotation = glm::vec3(0.0f),
		glm::vec3 scale = glm::vec3(1.0f)
	);
	Mesh(
		const Mesh& other
	);

	virtual ~Mesh();

	void Draw(Shader* shader);

	inline void SetPosition(const glm::vec3 val) { _position = val; }
	inline void SetOrigin(const glm::vec3 val){ _origin = val; }
	inline void SetRotation(const glm::vec3 val){ _rotation = val; }
	inline void SetScale(const glm::vec3 val){_scale = val; }

	inline void Translate(const glm::vec3 val){ _position += val; }
	inline void Rotate(const glm::vec3 val){ _rotation += val; }
	inline void Scale(const glm::vec3 val){ _scale += val; }

};