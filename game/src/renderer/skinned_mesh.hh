#pragma once

#include <string>
#include <vector>
#include <map>

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <glew.h> // Must be BEFORE GLFW!
#include <glfw3.h>

#include "renderer/model.hh"
#include "renderer/shader.hh"
#include "common.hh"

class SkinnedMesh
{
private:
#define NUM_BONES_PER_VERTEX 4

	struct BoneInfo
	{
		glm::mat4 BoneOffset;
		glm::mat4 FinalTransformation;

		BoneInfo()
		{
			BoneOffset = glm::mat4(0.0f);
			FinalTransformation = glm::mat4(0.0f);
		}
	};

	struct VertexBoneData
	{
		// To keep the number of items of IDs & weights
		unsigned int IDs[NUM_BONES_PER_VERTEX];
		float Weights[NUM_BONES_PER_VERTEX];

		VertexBoneData()
		{
			Reset();
		}

		void Reset()
		{
			for (unsigned int i = 0; i < NUM_BONES_PER_VERTEX; ++i)
			{
				IDs[i] = 0;
				Weights[i] = 0;
			}
		}

		void AddBoneData(unsigned int BoneID, float Weight);
	};

	void CalcInterpolatedScaling(aiVector3D& Out, float AnimationTime, const aiNodeAnim* pNodeAnim);
	void CalcInterpolatedRotation(aiQuaternion& Out, float AnimationTime, const aiNodeAnim* pNodeAnim);
	void CalcInterpolatedPosition(aiVector3D& Out, float AnimationTime, const aiNodeAnim* pNodeAnim);
	unsigned int FindScaling(float AnimationTime, const aiNodeAnim* pNodeAnim);
	unsigned int FindRotation(float AnimationTime, const aiNodeAnim* pNodeAnim);
	unsigned int FindPosition(float AnimationTime, const aiNodeAnim* pNodeAnim);
	const aiNodeAnim* FindNodeAnim(const aiAnimation* pAnimation, const std::string NodeName);
	void ReadNodeHeirarchy(float AnimationTime, const aiNode* pNode, const glm::mat4& ParentTransform);
	bool InitFromScene(const aiScene* pScene, const std::string& Filename);

	void InitMesh(
		unsigned int MeshIndex,
		const aiMesh* paiMesh,
		std::vector<glm::vec3>& Positions,
		std::vector<glm::vec3>& Colors,
		std::vector<glm::vec2>& TexCoords,
		std::vector<glm::vec3>& Normals,
		std::vector<VertexBoneData>& Bones,
		std::vector<GLuint>& Indices
	);

	void LoadBones(unsigned int MeshIndex, const aiMesh* paiMesh, std::vector<VertexBoneData>& Bones);
	bool InitMaterials(const aiScene* pScene, const std::string& Filename);
	void Clear();

	enum VB_TYPES {
		INDEX_BUFFER,
		POS_VB,
		COLOR_VB,
		TEXCOORD_VB,
		NORMAL_VB,
		BONE_VB,
		NUM_VBs
	};

	GLuint m_VAO;
	GLuint m_Buffers[NUM_VBs];

	struct MeshEntry
	{
		MeshEntry()
		{
			NumIndices = 0;
			BaseVertex = 0;
			BaseIndex = 0;
			MaterialIndex = 0xFFFFFFFF; // Initialize to an invalid index which is unlikely to be used
		}

		unsigned int NumIndices;
		unsigned int BaseVertex;
		unsigned int BaseIndex;
		unsigned int MaterialIndex;
	};

	std::vector<MeshEntry> m_Entries;
	std::vector<GLuint> m_Textures;

	std::map<std::string, unsigned int> m_BoneMapping; // maps a bone name to its index
	unsigned int m_NumBones;
	std::vector<BoneInfo> m_BoneInfo;
	glm::mat4 m_GlobalInverseTransform;

	/* duration of the animation, can be changed if frames are not present in all interval */
	double animDuration;

	unsigned int currentAnimation;

	const aiScene* m_pScene;
	Assimp::Importer m_Importer;
public:
	SkinnedMesh();
	~SkinnedMesh();

	unsigned int getNumAnimations();
	void setAnimation(unsigned int a);

	bool loadMesh(const std::string& fileName);

	void Draw(Shader* shader);

	unsigned int numBones() const
	{
		return m_NumBones;
	}

	void boneTransform(float timeInSeconds, std::vector<glm::mat4>& Transforms);
	void setBoneTransformations(Shader* shader, GLfloat currentTime);
};