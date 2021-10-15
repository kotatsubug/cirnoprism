#include "mesh.hh"

/// Use this to get an array of all current animated transformation matrices by bone ID.
std::vector<glm::mat4> Mesh::_GetBoneTransforms()
{
	std::vector<glm::mat4> boneMatrices;
	boneMatrices.resize(_numBones);
	_AddBonesToArray(_boneHierarchy, boneMatrices);
	return boneMatrices;
}

void Mesh::_AddBonesToArray(BoneTreeNode& bone, std::vector<glm::mat4>& boneMatrices)
{
	boneMatrices[bone.GetID()] = bone.GetAnimTransform();
	for (auto& childBone : bone.GetChildren())
	{
		_AddBonesToArray(childBone, boneMatrices);
	}
}

//Mesh::Mesh(
//	Vertex* vertices, const uint32_t& numVertices,
//	GLuint* indices, const uint32_t& numIndices,
//	glm::vec3 position,
//	glm::vec3 origin,
//	glm::vec3 rotation,
//	glm::vec3 scale
//)
//{
//	_numVertices = numVertices;
//	_numIndices = numIndices;
//
//	_position = position;
//	_origin = origin;
//	_rotation = rotation;
//	_scale = scale;
//
//	_vertices = new Vertex[_numVertices];
//	for (uint32_t i = 0; i < _numVertices; i++)
//	{
//		_vertices[i] = vertices[i];
//	}
//
//	_indices = new GLuint[_numIndices];
//	for (uint32_t i = 0; i < _numIndices; i++)
//	{
//		_indices[i] = indices[i];
//	}
//
//	// Order this is executed in is crucial
//	_InitMeshBuffers();
//	_UpdateModelMatrix();
//}
//
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

	_vertices = new PerVertexData[_numVertices];
	for (uint32_t i = 0; i < _numVertices; i++)
	{
		_vertices[i] = primitive->GetVertices()[i];
	}

	_indices = new GLuint[_numIndices];
	for (uint32_t i = 0; i < _numIndices; i++)
	{
		_indices[i] = primitive->GetIndices()[i];
	}
	
	_InitMeshBuffers();
	_UpdateModelMatrix();
}

Mesh::Mesh(
	const std::string& path,
	const uint16_t type,
	glm::vec3 position,
	glm::vec3 origin,
	glm::vec3 rotation,
	glm::vec3 scale)
	:
	_position(position),
	_origin(origin),
	_rotation(rotation),
	_scale(scale)
{
	std::vector<PerVertexData> vertices;
	if (type & 1)
	{
		ImportMD5Mesh(path, vertices, 0x0001);
	}
	else
	{
		ImportOBJ(path, vertices);
	}
	

	_numVertices = vertices.size();
	_numIndices = 0;

	_vertices = new PerVertexData[_numVertices];
	for (uint32_t i = 0; i < _numVertices; i++)
	{
		_vertices[i] = (vertices.data())[i];
	}

	_indices = NULL;

	
//	_boneHierarchy = BoneTreeNode(1, "A", glm::mat4(1.0f));
//	std::vector<glm::ivec4> boneIDs;
//	std::vector<glm::vec4> boneWeights;
//	ImportDAESkeleton(path, _boneHierarchy, _numBones);
	
	


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

	_vertices = new PerVertexData[_numVertices];
	for (uint32_t i = 0; i < _numVertices; i++)
	{
		_vertices[i] = other._vertices[i];
	}

	_indices = new GLuint[_numIndices];
	for (uint32_t i = 0; i < _numIndices; i++)
	{
		_indices[i] = other._indices[i];
	}

	_boneHierarchy = other._boneHierarchy;

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
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(PerVertexData), (GLvoid*)offsetof(PerVertexData, position));
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(PerVertexData), (GLvoid*)offsetof(PerVertexData, color));
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(PerVertexData), (GLvoid*)offsetof(PerVertexData, texcoord));
	glEnableVertexAttribArray(2);

	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(PerVertexData), (GLvoid*)offsetof(PerVertexData, normal));
	glEnableVertexAttribArray(3);

	// Unbind
	glBindVertexArray(0);

	// TODO: Error check
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

void Mesh::_UpdateAnimations()
{
	_animator.Update();
}

/*
/// Determines gBones array data based on current animation, time, etc.
/// Then sends that data into the shader as one giant uniform mat4 array.
void Mesh::_UpdateBoneTransformations(Shader* shader, GLfloat currentTime)
{
	// Get an array of needed bone transforms
	// Maximum number of bone transforms specified per shader properties
	std::vector<glm::mat4> transforms;
	_BoneTransform((float)currentTime, transforms);

	// Send to shader
	shader->SetArrMat4fv(transforms, "gBones");
}

/// Populates the gBones array fed into this function with relevant transformation matrices.
/// Uses given time to deduce from the current animation how to apply the
/// transforms within the node hierarchy of the mesh armature.
void Mesh::_BoneTransform(float timeInSeconds, std::vector<glm::mat4>& transforms)
{
	const glm::mat4 IDENTITY_MATRIX = glm::mat4(1.0f);

	//TODO: I think that this line does not make any sense... because its overwritten later
	//		Whatever the tutorial tells me to do I guess.
	_animDuration = (float)_pScene->mAnimations[_currentAnimation]->mDuration;

	// Calculate the animation duration
	uint32_t numPosKeys = _pScene->mAnimations[_currentAnimation]->mChannels[0]->mNumPositionKeys;
	_animDuration = _pScene->mAnimations[_currentAnimation]->mChannels[0]->mPositionKeys[numPosKeys - 1].mTime;

	float ticksPerSecond = (float)(_pScene->mAnimations[_currentAnimation]->mTicksPerSecond != 0
		? _pScene->mAnimations[_currentAnimation]->mTicksPerSecond : 25.0f);

	float timeInTicks = timeInSeconds * ticksPerSecond;
	float animationTime = fmod(timeInTicks, _animDuration);

	// Transform!
	_ReadNodeHierarchy(animationTime, _pScene->mRootNode, IDENTITY_MATRIX);
	transforms.resize(_numBones);
	for (uint32_t i = 0; i < _numBones; i++)
	{
		transforms[i] = _boneInfo[i].finalTransformation;
	}
}

void Mesh::_ReadNodeHierarchy(float animationTime, const aiNode* pNode, const glm::mat4& parentTransform)
{
	std::string nodeName(pNode->mName.data);
	const aiAnimation* pAnimation = _pScene->mAnimations[_currentAnimation];
	glm::mat4 NodeTransformation = mat4_cast(pNode->mTransformation);
	const aiNodeAnim* pNodeAnim = _FindNodeAnim(pAnimation, nodeName);

	if (pNodeAnim)
	{
		// Matrix multiplication goes the other way around. Order is crucial here.

		// Interpolate scaling and generate scaling transformation matrix
		aiVector3D Scaling;
		_CalcInterpolatedScale(Scaling, animationTime, pNodeAnim);
		glm::vec3 scale = glm::vec3(Scaling.x, Scaling.y, Scaling.z);
		glm::mat4 ScalingM = glm::scale(glm::mat4(1.0f), scale);

		// Interpolate rotation and generate rotation transformation matrix
		aiQuaternion RotationQ;
		_CalcInterpolatedRotation(RotationQ, animationTime, pNodeAnim);
		glm::quat rotation = quat_cast(RotationQ);
		glm::mat4 RotationM = glm::toMat4(rotation);

		// Interpolate translation and generate translation transformation matrix
		aiVector3D Translation;
		_CalcInterpolatedPosition(Translation, animationTime, pNodeAnim);
		glm::vec3 translation = glm::vec3(Translation.x, Translation.y, Translation.z);
		glm::mat4 TranslationM = glm::translate(glm::mat4(1.0f), translation);

		// Combine the above transformations
		NodeTransformation = TranslationM * RotationM * ScalingM;
	}

	// Combine with node Transformation with Parent Transformation
	glm::mat4 GlobalTransformation = parentTransform * NodeTransformation;

	if (_boneMap.find(nodeName) != _boneMap.end())
	{
		uint32_t boneIndex = _boneMap[nodeName];
		_boneInfo[boneIndex].finalTransformation = _globalInverseTransform * GlobalTransformation * _boneInfo[boneIndex].boneOffset;
	}

	for (uint32_t i = 0; i < pNode->mNumChildren; i++)
	{
		_ReadNodeHierarchy(animationTime, pNode->mChildren[i], GlobalTransformation);
	}
}

const aiNodeAnim* Mesh::_FindNodeAnim(const aiAnimation* pAnimation, const std::string nodeName)
{
	for (uint32_t i = 0; i < pAnimation->mNumChannels; i++)
	{
		const aiNodeAnim* pNodeAnim = pAnimation->mChannels[i];

		if (std::string(pNodeAnim->mNodeName.data) == nodeName)
		{
			return pNodeAnim;
		}
	}

	return NULL;
}*/

/// Should be called every frame to render this specific mesh.
/// Shader: which shader to throw this mesh's modelMatrix into when rendering.
void Mesh::Draw(Shader* shader)
{
	_UpdateModelMatrix();
	_UpdateUniforms(shader);
	_UpdateAnimations();
	
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