#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <map>

#include "renderer/vertex.hh"
#include "renderer/shader.hh"
#include "renderer/texture.hh"
#include "renderer/material.hh"
#include "renderer/primitives.hh"
#include "common.hh"

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

#include "math/math_linalg.hh"
#include "math/math_quat.hh"

#include "util/md5_importer.hh"
#include "util/obj_importer.hh"

#include "renderer/skeletal_animation.hh"

class Mesh
{
private:
	// OpenGL
	PerVertexData* _vertices; // Vertex array
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
	void _UpdateAnimations();

	
	BoneTreeNode _boneHierarchy; // Tree of all translation/rotation bone transforms
	uint32_t _numBones;

	Animator _animator;
	
	std::vector<glm::mat4> _GetBoneTransforms();
	void _AddBonesToArray(BoneTreeNode& bone, std::vector<glm::mat4>& boneMatrices);

public:
//	Mesh(
//		Vertex* vertices, const uint32_t& numVertices,
//		GLuint* indices, const uint32_t& numIndices,
//		glm::vec3 position = glm::vec3(0.0f),
//		glm::vec3 origin = glm::vec3(0.0f),
//		glm::vec3 rotation = glm::vec3(0.0f),
//		glm::vec3 scale = glm::vec3(1.0f)
//	);

	Mesh(
		Primitive* primitive,
		glm::vec3 position = glm::vec3(0.0f),
		glm::vec3 origin = glm::vec3(0.0f),
		glm::vec3 rotation = glm::vec3(0.0f),
		glm::vec3 scale = glm::vec3(1.0f)
	);

	Mesh(
		const std::string& path,
		const uint16_t type = 0x0000,
		glm::vec3 position = glm::vec3(0.0f),
		glm::vec3 origin = glm::vec3(0.0f),
		glm::vec3 rotation = glm::vec3(0.0f),
		glm::vec3 scale = glm::vec3(1.0f)
	);

	Mesh(const Mesh& other);

	virtual ~Mesh();

	void Draw(Shader* shader);

	inline void SetPosition(const glm::vec3 val) { _position = val; }
	inline void SetOrigin(const glm::vec3 val){ _origin = val; }
	inline void SetRotation(const glm::vec3 val){ _rotation = val; }
	inline void SetScale(const glm::vec3 val){ _scale = val; }

	inline void Translate(const glm::vec3 val){ _position += val; }
	inline void Rotate(const glm::vec3 val){ _rotation += val; }
	inline void Scale(const glm::vec3 val){ _scale += val; }


};