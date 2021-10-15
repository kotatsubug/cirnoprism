#pragma once

#include <glew.h>
#include <glfw3.h>

#include <vector>

#include "vertex.hh"

class Primitive
{
private:
	std::vector<PerVertexData> _vertices;
	std::vector<GLuint> _indices;
public:
	Primitive()
	{
	
	}

	virtual ~Primitive()
	{
	
	}

	// Functions
	void Set(
		const PerVertexData* vertices,
		const unsigned numberOfVertices,
		const GLuint* indices,
		const unsigned numberOfIndices)
	{
		for (size_t i = 0; i < numberOfVertices; i++)
		{
			_vertices.push_back(vertices[i]);
		}

		for (size_t i = 0; i < numberOfIndices; i++)
		{
			_indices.push_back(indices[i]);
		}
	}

	inline PerVertexData* GetVertices() { return _vertices.data(); }
	inline GLuint* GetIndices() { return _indices.data(); }
	inline unsigned GetNumberOfVertices() const { return _vertices.size(); }
	inline unsigned GetNumberOfIndices() const { return _indices.size(); }
};

class Quad : public Primitive
{
private:

public:
	Quad()
		: Primitive() // Also call the constructor
	{
		PerVertexData vertices[] =
		{
			// Position | Color | TexCoords | Normals | BoneIDs | BoneWeights
			glm::vec3(-0.5f, 0.5f, 0.0f),	glm::vec3(1.0f, 1.0f, 1.0f),	glm::vec2(0.0f, 1.0f),	glm::vec3(0.0f, 0.0f, 1.0f), glm::ivec4(0), glm::vec4(0.0f),
			glm::vec3(-0.5f, -0.5f, 0.0f),	glm::vec3(1.0f, 1.0f, 1.0f),	glm::vec2(0.0f, 0.0f),	glm::vec3(0.0f, 0.0f, 1.0f), glm::ivec4(0), glm::vec4(0.0f),
			glm::vec3(0.5f, -0.5f, 0.0f),	glm::vec3(1.0f, 1.0f, 1.0f),	glm::vec2(1.0f, 0.0f),	glm::vec3(0.0f, 0.0f, 1.0f), glm::ivec4(0), glm::vec4(0.0f),
			glm::vec3(0.5f, 0.5f, 0.0f),	glm::vec3(1.0f, 1.0f, 1.0f),	glm::vec2(1.0f, 1.0f),	glm::vec3(0.0f, 0.0f, 1.0f), glm::ivec4(0), glm::vec4(0.0f)
		};
		unsigned numberOfVertices = sizeof(vertices) / sizeof(PerVertexData);

		GLuint indices[] =
		{
			0, 1, 2,
			0, 2, 3
		};
		unsigned numberOfIndices = sizeof(indices) / sizeof(GLuint);

		this->Set(vertices, numberOfVertices, indices, numberOfIndices);
	}
};

class Pyramid : public Primitive
{
public:
	Pyramid()
		: Primitive()
	{
		PerVertexData vertices[] =
		{
			glm::vec3(0.0f, 0.5f, 0.0f),	glm::vec3(1.0f, 1.0f, 1.0f),	glm::vec2(0.5f, 1.0f),	glm::vec3(0.0f, 0.0f, 1.0f),  glm::ivec4(0), glm::vec4(0.0f),
			glm::vec3(-0.5f, -0.5f, 0.5f),	glm::vec3(1.0f, 1.0f, 1.0f),	glm::vec2(0.0f, 0.0f),	glm::vec3(0.0f, 0.0f, 1.0f),  glm::ivec4(0), glm::vec4(0.0f),
			glm::vec3(0.5f, -0.5f, 0.5f),	glm::vec3(1.0f, 1.0f, 1.0f),	glm::vec2(1.0f, 0.0f),	glm::vec3(0.0f, 0.0f, 1.0f),  glm::ivec4(0), glm::vec4(0.0f),
																																  
			glm::vec3(0.0f, 0.5f, 0.0f),	glm::vec3(1.0f, 1.0f, 1.0f),	glm::vec2(0.5f, 1.0f),	glm::vec3(-1.0f, 0.0f, 0.0f), glm::ivec4(0), glm::vec4(0.0f),
			glm::vec3(-0.5f, -0.5f, -0.5f),	glm::vec3(1.0f, 1.0f, 1.0f),	glm::vec2(0.0f, 0.0f),	glm::vec3(-1.0f, 0.0f, 0.0f), glm::ivec4(0), glm::vec4(0.0f),
			glm::vec3(-0.5f, -0.5f, 0.5f),	glm::vec3(1.0f, 1.0f, 1.0f),	glm::vec2(1.0f, 0.0f),	glm::vec3(-1.0f, 0.0f, 0.0f), glm::ivec4(0), glm::vec4(0.0f),

			glm::vec3(0.0f, 0.5f, 0.0f),	glm::vec3(1.0f, 1.0f, 1.0f),	glm::vec2(0.5f, 1.0f),	glm::vec3(0.0f, 0.0f, -1.0f), glm::ivec4(0), glm::vec4(0.0f),
			glm::vec3(0.5f, -0.5f, -0.5f),	glm::vec3(1.0f, 1.0f, 1.0f),	glm::vec2(0.0f, 0.0f),	glm::vec3(0.0f, 0.0f, -1.0f), glm::ivec4(0), glm::vec4(0.0f),
			glm::vec3(-0.5f, -0.5f, -0.5f),	glm::vec3(1.0f, 1.0f, 1.0f),	glm::vec2(1.0f, 0.0f),	glm::vec3(0.0f, 0.0f, -1.0f), glm::ivec4(0), glm::vec4(0.0f),

			glm::vec3(0.0f, 0.5f, 0.0f),	glm::vec3(1.0f, 1.0f, 1.0f),	glm::vec2(0.5f, 1.0f),	glm::vec3(1.0f, 0.0f, 0.0f),  glm::ivec4(0), glm::vec4(0.0f),
			glm::vec3(0.5f, -0.5f, 0.5f),	glm::vec3(1.0f, 1.0f, 1.0f),	glm::vec2(0.0f, 0.0f),	glm::vec3(1.0f, 0.0f, 0.0f),  glm::ivec4(0), glm::vec4(0.0f),
			glm::vec3(0.5f, -0.5f, -0.5f),	glm::vec3(1.0f, 1.0f, 1.0f),	glm::vec2(1.0f, 0.0f),	glm::vec3(1.0f, 0.0f, 0.0f),  glm::ivec4(0), glm::vec4(0.0f)
		};
		unsigned numberOfVertices = sizeof(vertices) / sizeof(PerVertexData);

		this->Set(vertices, numberOfVertices, nullptr, 0);
	}
};