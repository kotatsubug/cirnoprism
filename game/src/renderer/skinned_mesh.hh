#ifdef ESDEFJKSDFIJSDF
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

		/// Populates the IDs and weights of a VertexBoneData instance for some Vertex.
		void AddBoneData(unsigned int BoneID, float Weight)
		{
			for (unsigned int i = 0; i < NUM_BONES_PER_VERTEX; ++i)
			{
				if (Weights[i] == 0.0f)
//	TODO			if (Weights[i] < std::numeric_limits<float>::epsilon())
				{
					IDs[i] = BoneID;
					Weights[i] = Weight;
					return;
				}
			}
		}
	};

	struct PerVertexData
	{
		glm::vec3 position;
		glm::vec3 color;
		glm::vec2 texCoord;
		glm::vec3 normal;
		VertexBoneData bone; // Note: this contains 4 IDs and 4 weights for every vertex!
		// Indices should be stored as a separate array as GLuint*s
	};

	PerVertexData* vertices;
	GLuint* indices;

	void CalcInterpolatedScaling(aiVector3D& Out, float AnimationTime, const aiNodeAnim* pNodeAnim);
	void CalcInterpolatedRotation(aiQuaternion& Out, float AnimationTime, const aiNodeAnim* pNodeAnim);
	void CalcInterpolatedPosition(aiVector3D& Out, float AnimationTime, const aiNodeAnim* pNodeAnim);
	unsigned int FindScaling(float AnimationTime, const aiNodeAnim* pNodeAnim);
	unsigned int FindRotation(float AnimationTime, const aiNodeAnim* pNodeAnim);
	unsigned int FindPosition(float AnimationTime, const aiNodeAnim* pNodeAnim);
	const aiNodeAnim* FindNodeAnim(const aiAnimation* pAnimation, const std::string NodeName);
	void ReadNodeHeirarchy(float AnimationTime, const aiNode* pNode, const glm::mat4& ParentTransform);
	bool InitFromScene(const aiScene* pScene, const std::string& Filename);

	void InitMesh(unsigned int MeshIndex, const aiMesh* paiMesh, PerVertexData* vertices, GLuint* indices);

	void LoadBones(unsigned int MeshIndex, const aiMesh* paiMesh, PerVertexData* boneSource);
	bool InitMaterials(const aiScene* pScene, const std::string& Filename) noexcept;
	void Clear();

//	enum VB_TYPES {
//		INDEX_BUFFER,
//		POS_VB,
//		COLOR_VB,
//		TEXCOORD_VB,
//		NORMAL_VB,
//		BONE_VB,
//		NUM_VBs
//	};

	GLuint m_VAO;
//	GLuint m_Buffers[NUM_VBs];
	GLuint m_VBO;
	GLuint m_EBO;

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

	// ---------------------

	glm::vec3 _position;
	glm::vec3 _origin;
	glm::vec3 _rotation;
	glm::vec3 _scale;

	glm::mat4 _modelMatrix;

public:
	SkinnedMesh();
	SkinnedMesh(const SkinnedMesh& obj); // Copy constructor

	~SkinnedMesh();

	unsigned int getNumAnimations();
	void setAnimation(unsigned int a);

	bool loadMesh(const std::string& fileName);

	/// Render the mesh to shader.
	/// This should update the uniforms every time it's rendered so positions/rotations/etc can be updated.
	void Draw(Shader* shader);

	unsigned int numBones() const
	{
		return m_NumBones;
	}

	void boneTransform(float timeInSeconds, std::vector<glm::mat4>& Transforms);
	void setBoneTransformations(Shader* shader, GLfloat currentTime);

	// --------------------

	inline void SetPosition(const glm::vec3 val) { _position = val; }
	inline void SetOrigin(const glm::vec3 val) { _origin = val; }
	inline void SetRotation(const glm::vec3 val) { _rotation = val; }
	inline void SetScale(const glm::vec3 val) { _scale = val; }

	inline void Translate(const glm::vec3 val) { _position += val; }
	inline void TranslateOrigin(const glm::vec3 val) { _origin += val; }
	inline void Rotate(const glm::vec3 val) { _rotation += val; }
	inline void Scale(const glm::vec3 val) { _scale += val; }

};
#endif