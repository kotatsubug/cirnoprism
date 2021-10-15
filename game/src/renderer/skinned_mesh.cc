#ifdef ESDEFJKSDFIJSDF
#include "skinned_mesh.hh"

#define GLM_ENABLE_EXPERIMENTAL
#include <gtx\string_cast.hpp>
#include <gtx\quaternion.hpp>

// For converting between ASSIMP and glm
static inline glm::vec3 vec3_cast(const aiVector3D& v) { return glm::vec3(v.x, v.y, v.z); }
static inline glm::vec2 vec2_cast(const aiVector3D& v) { return glm::vec2(v.x, v.y); }
static inline glm::quat quat_cast(const aiQuaternion& q) { return glm::quat(q.w, q.x, q.y, q.z); }
static inline glm::mat4 mat4_cast(const aiMatrix4x4& m) { return glm::transpose(glm::make_mat4(&m.a1)); }
static inline glm::mat4 mat4_cast(const aiMatrix3x3& m) { return glm::transpose(glm::make_mat3(&m.a1)); }

void SkinnedMesh::setBoneTransformations(Shader* shader, GLfloat currentTime)
{
	std::vector<glm::mat4> Transforms;
	boneTransform((float)currentTime, Transforms);
	shader->SetMat4fv(Transforms[0], "gBones");
//	REMOVE: glUniformMatrix4fv(glGetUniformLocation(shader, "gBones"), (GLsizei)Transforms.size(), GL_FALSE, glm::value_ptr(Transforms[0]));
}

SkinnedMesh::SkinnedMesh()
	: currentAnimation(0)
{
	_position = glm::vec3(0.0f);
	_origin = glm::vec3(0.0f);
	_rotation = glm::vec3(0.0f);
	_scale = glm::vec3(1.0f);

	m_VAO = 0;
//	for (unsigned int i = 0; i < NUM_VBs; ++i)
//	{
	//	m_Buffers[i] = 0;
		m_VBO = 0;
//	}
	m_NumBones = 0;
	m_pScene = NULL;
}

/// Copy constructor
/// Do not use if LoadModel or other pScene-populating function hasn't been called
SkinnedMesh::SkinnedMesh(const SkinnedMesh& obj)
	: currentAnimation(0)
{
	m_VAO = obj.m_VAO;

//	memcpy(m_Buffers, obj.m_Buffers, sizeof(m_Buffers));
	m_VBO = obj.m_VBO;
	m_EBO = obj.m_EBO;

	m_Entries = obj.m_Entries;
	m_Textures = obj.m_Textures;
	m_BoneMapping = obj.m_BoneMapping;
	m_NumBones = obj.m_NumBones;
	m_BoneInfo = obj.m_BoneInfo;
	m_GlobalInverseTransform = obj.m_GlobalInverseTransform;
	animDuration = obj.animDuration;
	// currentAnimation resets to zero
	m_pScene = obj.m_pScene;

	_position = obj._position;
	_origin = obj._origin;
	_rotation = obj._rotation;
	_scale = obj._scale;

//	loadMesh("res/models/bob_lamp.md5mesh");
	
	// m_Importer shouldn't be needed since its only purpose is to populate m_pScene
	// Shouldn't be needed unless m_pScene is NULL for some reason before copy constructor is called
}

SkinnedMesh::~SkinnedMesh()
{
	Clear();
}

void SkinnedMesh::Clear()
{
	// Textures must not be deleted

	// Deletes VBOs
//	if (m_Buffers[0] != 0)
//	{
//		glDeleteBuffers(NUM_VBs, m_Buffers);
	if (m_VBO != 0)
	{
		glDeleteBuffers(1, &m_VBO);
		m_VBO = 0;
	}
//	}

	// Deletes VAO
	if (m_VAO != 0)
	{
		glDeleteVertexArrays(1, &m_VAO);
		m_VAO = 0;
	}

	// Delete EBO
	if (m_EBO != 0)
	{
		glDeleteBuffers(1, &m_EBO);
		m_EBO = 0;
	}

//	if (vertices.size() > 0)
	delete[] vertices;
	delete[] indices;
}

/// Loads the model.
bool SkinnedMesh::loadMesh(const std::string& fileName)
{
	/* Deletes the previous loaded mesh(if it exists) */
	Clear();

	/* Create the VAO */
//	glGenVertexArrays(1, &m_VAO);
//	glBindVertexArray(m_VAO);
	glCreateVertexArrays(1, &m_VAO);
	glBindVertexArray(m_VAO);

	/* Create VBOs for vertices attributes */
//	glGenBuffers(NUM_VBs, m_Buffers);
	glGenBuffers(1, &m_VBO);

	/* Return value */
	bool ret = false;

	m_pScene = m_Importer.ReadFile(fileName.c_str(), aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals);

	if (m_pScene)
	{
		/* Get transformation matrix for nodes(vertices relative to boes) */
		m_GlobalInverseTransform = mat4_cast(m_pScene->mRootNode->mTransformation);
		m_GlobalInverseTransform = glm::inverse(m_GlobalInverseTransform);

		ret = InitFromScene(m_pScene, fileName);
	}
	else
	{
		std::cout << "Error parsing : " << fileName << " : " << m_Importer.GetErrorString() << std::endl;
		return false;
	}

	/* Make sure the VAO is not changed from the outside */
	glBindVertexArray(0);

	return ret;
}

unsigned int SkinnedMesh::getNumAnimations()
{
	return m_pScene->mNumAnimations;
}

void SkinnedMesh::setAnimation(unsigned int a)
{
	if (a >= 0 && a < getNumAnimations())
	{
		currentAnimation = a;
	}
}

bool SkinnedMesh::InitFromScene(const aiScene* pScene, const std::string& fileName)
{
	/* Resize the mesh & texture vectors */
	m_Entries.resize(pScene->mNumMeshes);
	m_Textures.resize(pScene->mNumMaterials);

	// The original ASSIMP skeletal importer used here created multiple VBOs for
	// positions, texcoords, etc. as individual variables and initialized those buffers
	// one by one by enabling AttribArrays, using AttribPointers, and an array of VBO's
	// (VBO[0] = positions, VBO[1] = colors, etc...)

	// Instead of this, I'm making a struct of everything I need as one single big VBO
	// and enabling the AttribArray for the things I need in shaders.

	// (DEFINED IN HEADER)
//	std::vector<glm::vec3> Positions;
//	std::vector<glm::vec3> _colors;
//	std::vector<glm::vec2> TexCoords;
//	std::vector<glm::vec3> Normals;
//	std::vector<VertexBoneData> Bones;
//	std::vector<unsigned int> Indices;

	unsigned int numVertices = 0;
	unsigned int numIndices = 0;

	// Get count of vertices and indices
	for (unsigned int i = 0; i < m_Entries.size(); i++)
	{
		m_Entries[i].MaterialIndex = pScene->mMeshes[i]->mMaterialIndex;
		m_Entries[i].NumIndices = pScene->mMeshes[i]->mNumFaces * 3;
		m_Entries[i].BaseVertex = numVertices;
		m_Entries[i].BaseIndex = numIndices;

		numVertices += pScene->mMeshes[i]->mNumVertices;
		numIndices += m_Entries[i].NumIndices;
	}

	printf("Vertices: %i\n", numVertices);
	printf("Indices: %i\n", numIndices);

	// Reserve space in the vectors for the vertex attributes and indices
	vertices = new PerVertexData[468]; // 76 bytes/element. 1884 vertices throws MAV, 3080 is fine??
	indices = new GLuint[3080]; // 4 bytes/element
	if (vertices == nullptr) std::cout << "Memory could not be allocated\n";
	if (indices == nullptr) std::cout << "Memory could not be allocated\n";

//	for (unsigned int k = 0; k < 1884; k++)
//	{
//		vertices->bone.Reset();
//		printf("%f\n", vertices->bone.Weights[0]);
//	}

	//	vertices.position.reserve(numVertices);
	//	vertices.colors.reserve(numVertices);
	//	vertices.texCoords.reserve(numVertices);
	//	vertices.normals.reserve(numVertices);
	//	vertices.bones.resize(numVertices);
	//
	//	vertices.indices.reserve(numIndices);

	std::cout << "BEFORE INIT MESH [%i]" << numIndices << ".\n";
	for (unsigned int iter = 0; iter < numIndices; iter++)
	{
		printf("Indices [%i]: {%i, %i, %i}\n", iter, *(indices + 3 * iter), *(indices + 3 * iter + 1), *(indices + 3 * iter + 2));
	}

	// Initialize the meshes in the scene one by one
	for (unsigned int i = 0; i < m_Entries.size(); i++)
	{
		const aiMesh* paiMesh = pScene->mMeshes[i]; // Get mesh...
		InitMesh(i, paiMesh, vertices, indices); // ...feed mesh vertex buffer data into fVBO!
	}

	std::cout << "AFTER INIT MESH [%i]" << numIndices << ".\n";
	for (unsigned int iter = 0; iter < numIndices; iter++)
	{
		printf("Indices [%i]: {%i, %i, %i}\n", iter, *(indices + 3 * iter), *(indices + 3 * iter + 1), *(indices + 3 * iter + 2));
	}

	// Init the material
	if (!InitMaterials(pScene, fileName))
	{
		DEBUG_LOG("SkinnedMesh", LOG_ERROR, "Could not initialize materials from loaded scene!");
		return false;
	}

	// Generate and populate the buffers with vertex attributes and indices

	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
	glBufferData(GL_ARRAY_BUFFER, numVertices * sizeof(vertices[0]), vertices, GL_STATIC_DRAW);

	// if (numIndices > 0) {
	glGenBuffers(1, &m_EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, numIndices * sizeof(indices[0]), indices, GL_STATIC_DRAW);
	// }

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(PerVertexData), (GLvoid*)offsetof(PerVertexData, position));
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(PerVertexData), (GLvoid*)offsetof(PerVertexData, color));
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(PerVertexData), (GLvoid*)offsetof(PerVertexData, texCoord));
	glEnableVertexAttribArray(2);

	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(PerVertexData), (GLvoid*)offsetof(PerVertexData, normal));
	glEnableVertexAttribArray(3);

	glVertexAttribPointer(4, 4, GL_INT, GL_FALSE, sizeof(PerVertexData), (GLvoid*)offsetof(PerVertexData, bone.IDs)); // CONCERN
	glEnableVertexAttribArray(4);

	glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(PerVertexData), (GLvoid*)offsetof(PerVertexData, bone.Weights));
	glEnableVertexAttribArray(5);

	
	std::cout << "POSITION: " << vertices[0].position.x << ", " << vertices[0].position.y << ", " << vertices[0].position.z << "\n";
	std::cout << "COLOR: " << vertices[0].color.x << ", " << vertices[0].color.y << ", " << vertices[0].color.z << "\n";
	std::cout << "TEXCOORD: " << vertices[0].normal.x << ", " << vertices[0].normal.y << "\n";
	std::cout << "BONE ID: " <<
		vertices[0].bone.IDs[0] << ", "
		<< vertices[0].bone.IDs[1] << ", "
		<< vertices[0].bone.IDs[2] << ", "
		<< vertices[0].bone.IDs[3]
		<< "\n";
	std::cout << "BONE WEIGHT: " <<
		vertices[0].bone.Weights[0] << ", "
		<< vertices[0].bone.Weights[1] << ", "
		<< vertices[0].bone.Weights[2] << ", "
		<< vertices[0].bone.Weights[3]
		<< "\n";

	// These are all 1???
	for (unsigned int n = 0; n < numIndices; n++) std::cout << "Indice " << n << ": " << indices[0] << "\n";

/*	glBindBuffer(GL_ARRAY_BUFFER, m_Buffers[POS_VB]);																*/
/*	glBufferData(GL_ARRAY_BUFFER, sizeof(Positions[0]) * Positions.size(), &Positions[0], GL_STATIC_DRAW);			*/
/*	glEnableVertexAttribArray(0);																					*/
/*	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);															*/
/*																													*/
/*	glBindBuffer(GL_ARRAY_BUFFER, m_Buffers[COLOR_VB]);																*/
/*	glBufferData(GL_ARRAY_BUFFER, sizeof(_colors[0]) * _colors.size(), &_colors[0], GL_STATIC_DRAW);				*/
/*	glEnableVertexAttribArray(1);																					*/
/*	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);															*/
/*																													*/
/*	glBindBuffer(GL_ARRAY_BUFFER, m_Buffers[TEXCOORD_VB]);															*/
/*	glBufferData(GL_ARRAY_BUFFER, sizeof(TexCoords[0]) * TexCoords.size(), &TexCoords[0], GL_STATIC_DRAW);			*/
/*	glEnableVertexAttribArray(2);																					*/
/*	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);															*/
/*																													*/
/*	glBindBuffer(GL_ARRAY_BUFFER, m_Buffers[NORMAL_VB]);															*/
/*	glBufferData(GL_ARRAY_BUFFER, sizeof(Normals[0]) * Normals.size(), &Normals[0], GL_STATIC_DRAW);				*/
/*	glEnableVertexAttribArray(3);																					*/
/*	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, 0);															*/
/*																													*/
/*	glBindBuffer(GL_ARRAY_BUFFER, m_Buffers[BONE_VB]);																*/
/*	glBufferData(GL_ARRAY_BUFFER, sizeof(Bones[0]) * Bones.size(), &Bones[0], GL_STATIC_DRAW);						*/
/*	// Bone ID location																								*/
/*	glEnableVertexAttribArray(4);																					*/
/*	glVertexAttribIPointer(4, 4, GL_INT, sizeof(VertexBoneData), (const GLvoid*)0);									*/
/*	// Bone weight location																							*/
/*	glEnableVertexAttribArray(5);																					*/
/*	glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(VertexBoneData), (const GLvoid*)16);						*/
/*																													*/
/*	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_Buffers[INDEX_BUFFER]);													*/
/*	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Indices[0]) * Indices.size(), &Indices[0], GL_STATIC_DRAW);		*/

	return (glGetError() == GL_NO_ERROR);
}

void SkinnedMesh::InitMesh(unsigned int MeshIndex, const aiMesh* paiMesh, PerVertexData* vertices, GLuint* indices)
{
	const aiVector3D Zero3D(0.0f, 0.0f, 0.0f);

	std::cout << "Num vertices = " << paiMesh->mNumVertices << std::endl;
	/* Populize the vertex attribute vectors */
	for (unsigned int i = 0; i < paiMesh->mNumVertices; ++i)
	{
		/* Get pos normal texCoord */

		const aiVector3D* pPos = &(paiMesh->mVertices[i]);

		const aiVector3D* pTexCoord = paiMesh->HasTextureCoords(0) ? &(paiMesh->mTextureCoords[0][i]) : &Zero3D;

		/* store pos normal texCoord */
//		fVBO.vertices.push_back(glm::vec3(pPos->x, pPos->y, pPos->z));
		vertices[i].position = glm::vec3(pPos->x, pPos->y, pPos->z);

		if (paiMesh->HasNormals())
		{
			const aiVector3D* pNormal = &(paiMesh->mNormals[i]);

//			fVBO.normals.push_back(glm::vec3(pNormal->x, pNormal->y, pNormal->z));
			vertices[i].normal = glm::vec3(pNormal->x, pNormal->y, pNormal->z);
		}
//		fVBO.texCoords.push_back(glm::vec2(pTexCoord->x, pTexCoord->y));
		vertices[i].texCoord = glm::vec2(pTexCoord->x, pTexCoord->y);

		// TODO: colors
//		fVBO.colors.push_back(glm::vec3(1.0f));
		vertices[i].color = glm::vec3(1.0f);
	}

	// Load bones
	LoadBones(MeshIndex, paiMesh, vertices);

	// Populate index buffer
	for (unsigned int fn = 0; fn < paiMesh->mNumFaces; ++fn)
	{

		const aiFace& face = paiMesh->mFaces[fn];

		if (face.mNumIndices != 3)
		{
			DEBUG_LOG("SkinnedMesh", LOG_WARN, "Face [%i] has [%i] indices, expected 3!", fn, face.mNumIndices);
		}

	//	fVBO.indices.push_back(Face.mIndices[0]);
	//	fVBO.indices.push_back(Face.mIndices[1]);
	//	fVBO.indices.push_back(Face.mIndices[2]);
		std::cout << "Found {" << face.mIndices[0] << ", " << face.mIndices[1] << ", " << face.mIndices[2] << "}\n";

		indices[3*fn] = face.mIndices[0];
		indices[3*fn + 1] = face.mIndices[1];
		indices[3*fn + 2] = face.mIndices[2];

		std::cout << "Set as {" << indices[3 * fn] << ", " << indices[3 * fn + 1] << ", " << indices[3 * fn + 2] << "}\n";
	}
}

void SkinnedMesh::LoadBones(unsigned int MeshIndex, const aiMesh* pMesh, PerVertexData* boneSource)
{
	// For every bone,
	for (unsigned int i = 0; i < pMesh->mNumBones; ++i)
	{
		// Importing
		unsigned int BoneIndex = 0;
		std::string BoneName(pMesh->mBones[i]->mName.data);

		if (m_BoneMapping.find(BoneName) == m_BoneMapping.end())
		{
			/* allocate an index for the new bone */
			BoneIndex = m_NumBones;
			m_NumBones++;
			BoneInfo bi;
			m_BoneInfo.push_back(bi);

			m_BoneInfo[BoneIndex].BoneOffset = mat4_cast(pMesh->mBones[i]->mOffsetMatrix);
			m_BoneMapping[BoneName] = BoneIndex;
		}
		else
		{
			BoneIndex = m_BoneMapping[BoneName];
		}

		// Assigning
		for (unsigned int j = 0; j < pMesh->mBones[i]->mNumWeights; ++j)
		{
			unsigned int VertexID = m_Entries[MeshIndex].BaseVertex + pMesh->mBones[i]->mWeights[j].mVertexId;
			float Weight = pMesh->mBones[i]->mWeights[j].mWeight;
		//	Bones[VertexID].AddBoneData(BoneIndex, Weight);
			boneSource[VertexID].bone.AddBoneData(BoneIndex, Weight);
		}
	}
}

bool SkinnedMesh::InitMaterials(const aiScene* pScene, const std::string& Filename) noexcept
{
	// Extract the directory part from the file name
	std::string::size_type SlashIndex = Filename.find_last_of("/");
	std::string Dir;

	if (SlashIndex == std::string::npos)
	{
		Dir = ".";
	}
	else if (SlashIndex == 0)
	{
		Dir = "/";
	}
	else
	{
		Dir = Filename.substr(0, SlashIndex);
	}

	bool ret = true;

	/* Initialize the materials */
	for (unsigned int i = 0; i < pScene->mNumMaterials; ++i)
	{
		/* Get the material */
		const aiMaterial* pMaterial = pScene->mMaterials[i];

		m_Textures[i] = 0;

		if (pMaterial->GetTextureCount(aiTextureType_DIFFUSE) > 0)
		{
			aiString Path;

			if (pMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &Path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS)
			{
				std::string p(Path.data);

				if (p.substr(0, 2) == ".\\")
				{
					p = p.substr(2, p.size() - 2);
				}

				std::string FullPath = Dir + "/" + p;



				/* Here load the textures */
			}
		}
	}
	return ret;
}

void SkinnedMesh::Draw(Shader* shader)
{
	// Update model matrix ?
	// Update uniforms ? (shader)

	glBindVertexArray(m_VAO);

	for (unsigned int i = 0; i < m_Entries.size(); i++)
	{
		const uint64_t MaterialIndex = m_Entries[i].MaterialIndex;

		if (!(MaterialIndex < m_Textures.size()))
		{
			DEBUG_LOG("SkinnedMesh", LOG_ERROR, "Material index exceeds texture size!");
		}

	//	glDrawElements(GL_TRIANGLES, m_Entries[i].NumIndices, GL_UNSIGNED_INT, 0);
 		glDrawElementsBaseVertex(
			GL_TRIANGLES,
			m_Entries[i].NumIndices,
			GL_UNSIGNED_INT,
			(void*)(sizeof(unsigned int) * m_Entries[i].BaseIndex),
			m_Entries[i].BaseVertex
		);
	}

	glBindVertexArray(0);
}

unsigned int SkinnedMesh::FindPosition(float AnimationTime, const aiNodeAnim* pNodeAnim)
{
	for (unsigned int i = 0; i < pNodeAnim->mNumPositionKeys - 1; i++)
	{
		if (AnimationTime < (float)pNodeAnim->mPositionKeys[i + 1].mTime)
		{
			return i;
		}
	}

	return 0;
}

unsigned int SkinnedMesh::FindRotation(float AnimationTime, const aiNodeAnim* pNodeAnim)
{
	assert(pNodeAnim->mNumRotationKeys > 0);

	for (unsigned int i = 0; i < pNodeAnim->mNumRotationKeys - 1; i++)
	{
		if (AnimationTime < (float)pNodeAnim->mRotationKeys[i + 1].mTime)
		{
			return i;
		}
	}

	return 0;
}

unsigned int SkinnedMesh::FindScaling(float AnimationTime, const aiNodeAnim* pNodeAnim)
{
	assert(pNodeAnim->mNumScalingKeys > 0);

	for (unsigned int i = 0; i < pNodeAnim->mNumScalingKeys - 1; i++)
	{
		if (AnimationTime < (float)pNodeAnim->mScalingKeys[i + 1].mTime)
		{
			return i;
		}
	}

	return 0;
}

void SkinnedMesh::CalcInterpolatedPosition(aiVector3D& Out, float AnimationTime, const aiNodeAnim* pNodeAnim)
{
	if (pNodeAnim->mNumPositionKeys == 1)
	{
		Out = pNodeAnim->mPositionKeys[0].mValue;
		return;
	}

	unsigned int PositionIndex = FindPosition(AnimationTime, pNodeAnim);
	unsigned int NextPositionIndex = (PositionIndex + 1);
	assert(NextPositionIndex < pNodeAnim->mNumPositionKeys);
	float DeltaTime = (float)(pNodeAnim->mPositionKeys[NextPositionIndex].mTime - pNodeAnim->mPositionKeys[PositionIndex].mTime);
	float Factor = (AnimationTime - (float)pNodeAnim->mPositionKeys[PositionIndex].mTime) / DeltaTime;
	assert(Factor >= 0.0f && Factor <= 1.0f);
	const aiVector3D& Start = pNodeAnim->mPositionKeys[PositionIndex].mValue;
	const aiVector3D& End = pNodeAnim->mPositionKeys[NextPositionIndex].mValue;
	aiVector3D Delta = End - Start;
	Out = Start + Factor * Delta;
}

void SkinnedMesh::CalcInterpolatedRotation(aiQuaternion& Out, float AnimationTime, const aiNodeAnim* pNodeAnim)
{
	// we need at least two values to interpolate...
	if (pNodeAnim->mNumRotationKeys == 1) {
		Out = pNodeAnim->mRotationKeys[0].mValue;
		return;
	}

	unsigned int RotationIndex = FindRotation(AnimationTime, pNodeAnim);
	unsigned int NextRotationIndex = (RotationIndex + 1);
	assert(NextRotationIndex < pNodeAnim->mNumRotationKeys);
	float DeltaTime = (float)(pNodeAnim->mRotationKeys[NextRotationIndex].mTime - pNodeAnim->mRotationKeys[RotationIndex].mTime);
	float Factor = (AnimationTime - (float)pNodeAnim->mRotationKeys[RotationIndex].mTime) / DeltaTime;
	assert(Factor >= 0.0f && Factor <= 1.0f);
	const aiQuaternion& StartRotationQ = pNodeAnim->mRotationKeys[RotationIndex].mValue;
	const aiQuaternion& EndRotationQ = pNodeAnim->mRotationKeys[NextRotationIndex].mValue;
	aiQuaternion::Interpolate(Out, StartRotationQ, EndRotationQ, Factor);
	Out = Out.Normalize();
}

void SkinnedMesh::CalcInterpolatedScaling(aiVector3D& Out, float AnimationTime, const aiNodeAnim* pNodeAnim)
{
	if (pNodeAnim->mNumScalingKeys == 1) {
		Out = pNodeAnim->mScalingKeys[0].mValue;
		return;
	}

	unsigned int ScalingIndex = FindScaling(AnimationTime, pNodeAnim);
	unsigned int NextScalingIndex = (ScalingIndex + 1);
	assert(NextScalingIndex < pNodeAnim->mNumScalingKeys);
	float DeltaTime = (float)(pNodeAnim->mScalingKeys[NextScalingIndex].mTime - pNodeAnim->mScalingKeys[ScalingIndex].mTime);
	float Factor = (AnimationTime - (float)pNodeAnim->mScalingKeys[ScalingIndex].mTime) / DeltaTime;
	assert(Factor >= 0.0f && Factor <= 1.0f);
	const aiVector3D& Start = pNodeAnim->mScalingKeys[ScalingIndex].mValue;
	const aiVector3D& End = pNodeAnim->mScalingKeys[NextScalingIndex].mValue;
	aiVector3D Delta = End - Start;
	Out = Start + Factor * Delta;
}

void SkinnedMesh::ReadNodeHeirarchy(float AnimationTime, const aiNode* pNode, const glm::mat4& ParentTransform)
{
	std::string NodeName(pNode->mName.data);

	const aiAnimation* pAnimation = m_pScene->mAnimations[currentAnimation];

	glm::mat4 NodeTransformation = mat4_cast(pNode->mTransformation);

	const aiNodeAnim* pNodeAnim = FindNodeAnim(pAnimation, NodeName);

	if (pNodeAnim)
	{
		// Interpolate scaling and generate scaling transformation matrix
		aiVector3D Scaling;
		CalcInterpolatedScaling(Scaling, AnimationTime, pNodeAnim);
		glm::vec3 scale = glm::vec3(Scaling.x, Scaling.y, Scaling.z);
		glm::mat4 ScalingM = glm::scale(glm::mat4(1.0f), scale);

		// Interpolate rotation and generate rotation transformation matrix
		aiQuaternion RotationQ;
		CalcInterpolatedRotation(RotationQ, AnimationTime, pNodeAnim);
		glm::quat rotation = quat_cast(RotationQ);
		glm::mat4 RotationM = glm::toMat4(rotation);

		// Interpolate translation and generate translation transformation matrix
		aiVector3D Translation;
		CalcInterpolatedPosition(Translation, AnimationTime, pNodeAnim);
		glm::vec3 translation = glm::vec3(Translation.x, Translation.y, Translation.z);
		glm::mat4 TranslationM = glm::translate(glm::mat4(1.0f), translation);

		// Combine the above transformations
		NodeTransformation = TranslationM * RotationM * ScalingM;
	}

	// Combine with node Transformation with Parent Transformation
	glm::mat4 GlobalTransformation = ParentTransform * NodeTransformation;

	if (m_BoneMapping.find(NodeName) != m_BoneMapping.end())
	{
		unsigned int BoneIndex = m_BoneMapping[NodeName];
		m_BoneInfo[BoneIndex].FinalTransformation = m_GlobalInverseTransform * GlobalTransformation * m_BoneInfo[BoneIndex].BoneOffset;
	}

	for (unsigned int i = 0; i < pNode->mNumChildren; i++)
	{
		ReadNodeHeirarchy(AnimationTime, pNode->mChildren[i], GlobalTransformation);
	}
}

void SkinnedMesh::boneTransform(float timeInSeconds, std::vector<glm::mat4>& Transforms)
{
	glm::mat4 Identity = glm::mat4(1.0f);

	//TODO: I think that this line does not make any sense... because its overwritten later
	animDuration = (float)m_pScene->mAnimations[currentAnimation]->mDuration;

	/* Calc animation duration */
	unsigned int numPosKeys = m_pScene->mAnimations[currentAnimation]->mChannels[0]->mNumPositionKeys;
	animDuration = m_pScene->mAnimations[currentAnimation]->mChannels[0]->mPositionKeys[numPosKeys - 1].mTime;

	float TicksPerSecond = (float)(m_pScene->mAnimations[currentAnimation]->mTicksPerSecond != 0 ? m_pScene->mAnimations[currentAnimation]->mTicksPerSecond : 25.0f);
	//TicksPerSecond = 3;
	float TimeInTicks = timeInSeconds * TicksPerSecond;
	float AnimationTime = fmod(TimeInTicks, animDuration);

	ReadNodeHeirarchy(AnimationTime, m_pScene->mRootNode, Identity);

	Transforms.resize(m_NumBones);

	for (unsigned int i = 0; i < m_NumBones; i++)
	{
		Transforms[i] = m_BoneInfo[i].FinalTransformation;
	}
}

const aiNodeAnim* SkinnedMesh::FindNodeAnim(const aiAnimation* pAnimation, const std::string NodeName)
{
	for (unsigned int i = 0; i < pAnimation->mNumChannels; i++) {
		const aiNodeAnim* pNodeAnim = pAnimation->mChannels[i];

		if (std::string(pNodeAnim->mNodeName.data) == NodeName) {
			return pNodeAnim;
		}
	}

	return NULL;
}


// -----------------------


#endif