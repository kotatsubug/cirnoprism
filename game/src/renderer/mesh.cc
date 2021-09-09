#include "mesh.hh"

Mesh::Mesh(
	Vertex* vertices, const uint32_t& numVertices,
	GLuint* indices, const uint32_t& numIndices,
	glm::vec3 position,
	glm::vec3 origin,
	glm::vec3 rotation,
	glm::vec3 scale
)
{
	_numVertices = numVertices;
	_numIndices = numIndices;

	_position = position;
	_origin = origin;
	_rotation = rotation;
	_scale = scale;

	_vertices = new Vertex[_numVertices];
	for (uint32_t i = 0; i < _numVertices; i++)
	{
		_vertices[i] = vertices[i];
	}

	_indices = new GLuint[_numIndices];
	for (uint32_t i = 0; i < _numIndices; i++)
	{
		_indices[i] = indices[i];
	}

	// Order this is executed in is crucial
	_InitMeshBuffers();
	_UpdateModelMatrix();
}

Mesh::Mesh(
	Primitive* primitive,
	glm::vec3 position,
	glm::vec3 origin,
	glm::vec3 rotation,
	glm::vec3 scale
)
{
	_numVertices = primitive->GetNumberOfVertices();
	_numIndices = primitive->GetNumberOfIndices();
	
	_position = position;
	_origin = origin;
	_rotation = rotation;
	_scale = scale;

	_vertices = new Vertex[_numVertices];
	for (uint32_t i = 0; i < _numVertices; i++)
	{
		_vertices[i] = primitive->GetVertices()[i];
	}

	_indices = new GLuint[_numIndices];
	for (uint32_t i = 0; i < _numIndices; i++)
	{
		_indices[i] = primitive->GetIndices()[i];
	}

	// Order this is executed in is crucial
	_InitMeshBuffers();
	_UpdateModelMatrix();
}

Mesh::Mesh(const Mesh& other)
{
	_position = other._position;
	_origin = other._origin;
	_rotation = other._rotation;
	_scale = other._scale;

	_numVertices = other._numVertices;
	_numIndices = other._numIndices;

	_vertices = new Vertex[_numVertices];
	for (uint32_t i = 0; i < _numVertices; i++)
	{
		_vertices[i] = other._vertices[i];
	}

	_indices = new GLuint[_numIndices];
	for (uint32_t i = 0; i < _numIndices; i++)
	{
		_indices[i] = other._indices[i];
	}

	// Order this is executed in is crucial
	_InitMeshBuffers();
	_UpdateModelMatrix();
}

Mesh::~Mesh()
{
	glDeleteVertexArrays(1, &_vertexArrayBuffer);
	glDeleteBuffers(1, &_vertexArrayBuffer);
	if (_numIndices > 0)
	{
		glDeleteBuffers(1, &_elementArrayBuffer);
	}

	delete[] _vertices;
	delete[] _indices;
}

void Mesh::_InitMeshBuffers()
{
	// Create and bind VAO
	glCreateVertexArrays(1, &_vertexArrayObject);
	glBindVertexArray(_vertexArrayObject);

	// Create, bind, and send data of a VBO
	glGenBuffers(1, &_vertexArrayBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, _vertexArrayBuffer);
	glBufferData(GL_ARRAY_BUFFER, _numVertices * sizeof(_vertices[0]), _vertices, GL_STATIC_DRAW);

	// Create, bind, and send data of an EBO (if indices exist)
	if (_numIndices > 0)
	{
		glGenBuffers(1, &_elementArrayBuffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _elementArrayBuffer);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, _numIndices * sizeof(_indices[0]), _indices, GL_STATIC_DRAW);
	}

	// Set Vertex attribute pointers, then enable them at their specified location
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, position));
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, color));
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, texcoord));
	glEnableVertexAttribArray(2);

	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, normal));
	glEnableVertexAttribArray(3);

	// Unbind
	glBindVertexArray(0);
}

void Mesh::_UpdateModelMatrix()
{
	// When rotating any mesh in world space, it must be moved to the origin point, rotated, then MOVED BACK.
	// This is to assure the object rotates around its own relative origin, instead of always rotating around WORLD origin.
	// So every mesh, if rotated, will at one point be at the center of the world. Reality is a lie :^)
	_modelMatrix = glm::mat4(1.0f);
	_modelMatrix = glm::translate(_modelMatrix, _origin);
	_modelMatrix = glm::rotate(_modelMatrix, glm::radians(_rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
	_modelMatrix = glm::rotate(_modelMatrix, glm::radians(_rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
	_modelMatrix = glm::rotate(_modelMatrix, glm::radians(_rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
	_modelMatrix = glm::translate(_modelMatrix, _position - _origin);
	_modelMatrix = glm::scale(_modelMatrix, _scale);
}

void Mesh::_UpdateUniforms(Shader* shader)
{
	shader->SetMat4fv(_modelMatrix, "modelMatrix");
}

/// <summary>
/// Should be called every frame to render this specific mesh.
/// </summary>
/// <param name="shader">- which shader to throw this mesh's modelMatrix into when rendering.</param>
void Mesh::Draw(Shader* shader)
{
	_UpdateModelMatrix();
	_UpdateUniforms(shader);
	
	glBindVertexArray(_vertexArrayObject);

	if (_numIndices > 0)
	{
		glDrawElements(GL_TRIANGLES, _numIndices, GL_UNSIGNED_INT, 0);
	}
	else
	{
		glDrawArrays(GL_TRIANGLES, 0, _numVertices);
	}

	glBindVertexArray(0);
}
