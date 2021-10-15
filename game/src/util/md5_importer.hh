#pragma once

#include <vector>
#include <map>
#include <tuple>
#include <unordered_map>
#include <string>

#include <sstream>
#include <fstream>
#include <filesystem> // Only needed for file existence checking

#include <glm.hpp>
#include <vec3.hpp>
#include <vec4.hpp>
#include <mat4x4.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

#include "common.hh"

#include "renderer/vertex.hh"
#include "math/math_quat.hh"

#define DUPLICATE_VERTICES_TO_MATCH_INDEX_BUFFER_SIZE 0x0001

/// To be used when needing to discard garbage values read from sstream.
/// Example: "93.26000000 -1 10.51800000" -> std::cin >> a >> ss_ignore<int> >> b;
/// 
template <typename T = std::string, typename CharT = char>
static std::basic_istream<CharT>& IgnoreStringStream(std::basic_istream<CharT>& in)
{
	T ignoredValue;
	return in >> ignoredValue;
}

/// Parses an .mdl5mesh and populates vertex and index data including bone IDs and weights.
/// Exported meshes should have UP +Y. Reconstructs vertices to fit a single-index buffer
/// i.e. does not give a dynamic index buffer, only PerVertexData vertices!
/// NOTE: md5 does not natively contain vertex positions. They are calculated using the weights from the bind pose!
/// Do NOT export md5mesh with applied armature modifiers as they will screw up the bind pose by offsetting default vertex positions.
static bool ImportMD5Mesh(
	const std::string& path,
	std::vector<PerVertexData>& verticesOut,
	const uint16_t& flags = 0x0001) noexcept
{
	std::stringstream linestream;
	std::ifstream in_file(path);

	if (!std::filesystem::exists(path))
	{
		DEBUG_LOG("MD5Importer", LOG_ERROR, "Could not open '%s' (file does not exist)", path.c_str());
		return false;
	}

	if (!in_file.is_open())
	{
		DEBUG_LOG("MD5Importer", LOG_ERROR, "Could not open '%s' (file could not be read)", path.c_str());
		return false;
	}

	std::string line = "";
	std::string prefix = "";

	{
		bool flag = false;

		in_file.seekg(0, std::ios::beg); // Send stream to SOF
		while (std::getline(in_file, line) && !flag)
		{
			linestream.clear();
			linestream.str(line);
			linestream >> prefix;

			if (!prefix.compare("MD5Version"))
			{
				uint32_t version;
				linestream >> version;
				if (version != 10)
				{
					DEBUG_LOG("MD5Importer", LOG_WARN, "Expected MD5Version 10, got %u. There may be import errors!", version);
				}
			}
			else if (!prefix.compare("numMeshes"))
			{
				uint32_t n;
				linestream >> n;
				if (n > 1)
				{
					DEBUG_LOG("MD5Importer", LOG_WARN, "Expected numMeshes to be 1, got %u. There may be import errors!", n);
				}
				flag = true;
			}
		}
	}

	// While the memory can be easily contiguous here by sorted vectors,
	// I'm using hash maps because lookup by int is generally faster with hundreds of elements
	std::unordered_map<uint32_t, std::pair<uint32_t, uint32_t>> weightIndices; // [vertexID] -> {startWeight, numWeightsForThisVertex}
	std::unordered_map<uint32_t, std::tuple<std::string, int32_t, glm::fvec3, qt::Quaternion>> bones; // [bone ID] -> {name, parent bone ID, pos, rot}
	std::unordered_map<uint32_t, glm::fvec2> texCoords;
	std::unordered_map<uint32_t, std::vector<uint32_t>> triIndices; // [triID] -> {index i, index i+1, index i+2}

	std::vector<std::tuple<uint32_t, float, glm::fvec3>> weights; // {bone ID assoc. w/ the weight, bias [0.0,1.0], position XYZ}
	std::vector<glm::fvec3> vertPositions; // Calculated positions in bind pose
	std::vector<glm::fvec3> vertNormals;

	// Map bone name, parent bone ID, bone position, and bone rotation by bone ID
	{
		std::string tmpBoneName;
		int32_t tmpParentBoneID;
		glm::fvec3 tmpPos;
		float tmpRotX, tmpRotY, tmpRotZ;

		bool collectJoints = false;
		uint32_t boneIndex = 0;

		in_file.clear(); // Clear previous EOF bit, if reached
		in_file.seekg(0, std::ios::beg); // Send stream to SOF
		while (std::getline(in_file, line))
		{
			linestream.clear();
			linestream.str(line);
			linestream >> std::quoted(prefix); // Catch bone names in quotes

			if (collectJoints)
			{
				if (!prefix.compare("}"))
				{
					break;
				}

				tmpBoneName = prefix;
				linestream >> tmpParentBoneID 
					>> IgnoreStringStream<char>
					>> tmpPos.x >> tmpPos.y >> tmpPos.z
					>> IgnoreStringStream<char> >> IgnoreStringStream<char>
					>> tmpRotX >> tmpRotY >> tmpRotZ;

				bones.insert(std::make_pair(
					boneIndex,
					std::make_tuple(tmpBoneName, tmpParentBoneID, tmpPos, qt::Quaternion(tmpRotX, tmpRotY, tmpRotZ))
				));

				boneIndex++;
			}
			else if (!prefix.compare("joints"))
			{
				collectJoints = true;
			}
		}
	}

#ifdef IMPORTER_DEBUG
	for (const auto& [key, value] : bones)
	{
		DEBUG_LOG("MD5Importer", LOG_INFO, "bone ID %i, name [%s], has parent %i and location [%f,%f,%f] and rotation [%f,%f,%f]",
			key,
			std::get<0>(value).c_str(),
			std::get<1>(value),
			std::get<2>(value).x,
			std::get<2>(value).y,
			std::get<2>(value).z,
			std::get<3>(value)._x,
			std::get<3>(value)._y,
			std::get<3>(value)._z);
	}
#endif

	uint32_t numVertices = 0;
	uint32_t numTris;

	// Map texture coordinates and weight indices by vertex ID
	{
		uint32_t tmpVertID;
		uint32_t tmpTriIndex, tmpTriValue;
		glm::fvec2 tmpTexCoord;
		uint32_t tmpStartWeight, tmpNumWeights;
		bool collectMeshData = false;

		in_file.clear(); // Clear previous EOF bit, if reached
		in_file.seekg(0, std::ios::beg); // Send stream to SOF
		while (std::getline(in_file, line))
		{
			linestream.clear();
			linestream.str(line);
			linestream >> prefix;

			if (collectMeshData)
			{
				if (!prefix.compare("numverts"))
				{
					linestream >> numVertices;
#ifdef IMPORTER_DEBUG
					DEBUG_LOG("MD5Importer", LOG_INFO, "Got numverts=%i", numVertices);
#endif
				}
				else if (!prefix.compare("numtris"))
				{
					linestream >> numTris;
#ifdef IMPORTER_DEBUG
					DEBUG_LOG("MD5Importer", LOG_INFO, "Got numtris=%i", numTris);
#endif
				}
				else if (!prefix.compare("vert"))
				{
					linestream >> tmpVertID >> IgnoreStringStream<char> >> tmpTexCoord.x >> tmpTexCoord.y >> IgnoreStringStream<char> >> tmpStartWeight >> tmpNumWeights;

					tmpTexCoord.y = 1.0f - tmpTexCoord.y; // MD5 st(0,0) is in upper-left like DDS, OpenGL st(0,0) is lower-left!
					texCoords.insert(std::make_pair(
						tmpVertID, tmpTexCoord
					));

					weightIndices.insert(std::make_pair(
						tmpVertID, std::make_pair(tmpStartWeight, tmpNumWeights)
					));
				}
				else if (!prefix.compare("tri"))
				{
					linestream >> tmpTriIndex;
					for (uint32_t i = 0; i < 3; i++)
					{
						linestream >> tmpTriValue;
						triIndices[tmpTriIndex].push_back(tmpTriValue);
					}
				}
				else if (!prefix.compare("}"))
				{
					break;
				}
			}
			else if (!prefix.compare("mesh"))
			{
				collectMeshData = true;
			}
		}
	}

	if (numVertices == 0)
	{
		DEBUG_LOG("MD5Importer", LOG_ERROR, "Failed to find numverts...");
		in_file.close();
		return false;
	}

	if (numTris == 0)
	{
		DEBUG_LOG("MD5Importer", LOG_ERROR, "Failed to find numtris...");
		in_file.close();
		return false;
	}

#ifdef IMPORTER_DEBUG
	for (uint32_t i = 0; i < numVertices; i++)
	{
		DEBUG_LOG("MD5Importer", LOG_INFO, "vert %i has texcoord [%f,%f]", i, texCoords[i].x, texCoords[i].y);
	}
	for (uint32_t i = 0; i < weightIndices.size(); i++)
	{
		DEBUG_LOG("MD5Importer", LOG_INFO, "vert %i has startWeight [%i] and numWeights [%i]", i, weightIndices[i].first, weightIndices[i].second);
	}
#endif

	// To calculate vertex positions, weights are needed
	{
		uint32_t tmpBoneID;
		float tmpBias;
		glm::fvec3 tmpWeightPos;

		in_file.seekg(0, std::ios::beg); // Send stream to SOF
		while (std::getline(in_file, line))
		{
			linestream.clear();
			linestream.str(line);
			linestream >> prefix;

			if (!prefix.compare("weight"))
			{
				linestream >> IgnoreStringStream<uint32_t> >> tmpBoneID >> tmpBias >> IgnoreStringStream<char> >> tmpWeightPos.x >> tmpWeightPos.y >> tmpWeightPos.z;

				// Assuming weight indices are all sorted
				weights.push_back(std::make_tuple(tmpBoneID, tmpBias, tmpWeightPos));
			}
		}
	}

#ifdef IMPORTER_DEBUG
	for (uint32_t i = 0; i < weights.size(); i++)
	{
		DEBUG_LOG("MD5Importer", LOG_INFO, "weight ID %i is associated with bone ID %i, has bias %f, and position [%f, %f, %f]",
			i,
			std::get<0>(weights[i]),
			std::get<1>(weights[i]),
			std::get<2>(weights[i]).x,
			std::get<2>(weights[i]).y,
			std::get<2>(weights[i]).z);
	}
#endif

	// Calculate vertex positions
	for (uint32_t i = 0; i < numVertices; i++)
	{
		glm::fvec3 position = { 0.0f, 0.0f, 0.0f };

		// position(X,Y,Z) = Sum k=[0, N) in (bone[k].pos * weight[k].bias)
		// where N is the number of weights associated with this vertex in particular
		// and all bias values add up to 1.0 per vertex
		for (uint32_t j = 0; j < weightIndices[i].second; j++)
		{
			const auto& weight = weights[weightIndices[i].first + j];
			const auto& bone = bones[std::get<0>(weight)];

			// Calculate transformed vertex for this weight
			glm::fvec3 tVert = qt::Quaternion::RotatePoint(std::get<glm::fvec3>(weight), std::get<qt::Quaternion>(bone));

			// The sum of all weight->bias should be 1.0
			position.x += (std::get<glm::fvec3>(bone).x + tVert.x) * std::get<float>(weight);
			position.y += (std::get<glm::fvec3>(bone).y + tVert.y) * std::get<float>(weight);
			position.z += (std::get<glm::fvec3>(bone).z + tVert.z) * std::get<float>(weight);
		}

		vertPositions.push_back(position);
	}

#ifdef IMPORTER_DEBUG
	for (uint32_t i = 0; i < numVertices; i++)
	{
		DEBUG_LOG("MD5Importer", LOG_INFO, "Calculated vertex position %u [%f,%f,%f]",
			i, vertPositions[i].x, vertPositions[i].y, vertPositions[i].z);
	}
#endif

	// Calculate vertex normals
	// These must be recalculated during animation

	// First, initialize all vertex normals to 0
	for (uint32_t i = 0; i < numVertices; i++)
	{
		vertNormals.push_back(glm::vec3(0.0f, 0.0f, 0.0f));
	}

	// Then, for every unique triangle, calculate the face normal for that specific triangle
	// Add the face normal to the existing vertex normals for all three vertices, so that by the end,
	// each vertex normal is the unnormalized arithmetic mean of the triangle normals
	for (uint32_t t = 0; t < numTris; t++)
	{
		const glm::vec3 a = vertPositions[(triIndices[t])[0]];
		const glm::vec3 b = vertPositions[(triIndices[t])[1]];
		const glm::vec3 c = vertPositions[(triIndices[t])[2]];

		const glm::vec3 tangent = b - a;
		const glm::vec3 bitangent = c - a;
		const glm::vec3 normal = -glm::cross(tangent, bitangent);

		vertNormals[(triIndices[t])[0]] += normal;
		vertNormals[(triIndices[t])[1]] += normal;
		vertNormals[(triIndices[t])[2]] += normal;
	}

	// Finally, normalize all vertex normals
	for (uint32_t i = 0; i < numVertices; i++)
	{
		const float magnitude = static_cast<float>(sqrt(vertNormals[i].x*vertNormals[i].x + vertNormals[i].y*vertNormals[i].y + vertNormals[i].z*vertNormals[i].z));
		vertNormals[i] /= magnitude;
	}
	
#ifdef IMPORTER_DEBUG
	for (uint32_t i = 0; i < numVertices; i++)
	{
		DEBUG_LOG("MD5Importer", LOG_INFO, "Calculated normal [%i]: [%f, %f, %f]",
			i, vertNormals[i].x, vertNormals[i].y, vertNormals[i].z);
	}
#endif

	if (flags & DUPLICATE_VERTICES_TO_MATCH_INDEX_BUFFER_SIZE)
	{
		// Sort vertex indices from triIndices
		std::vector<uint32_t> vertIndices;
		for (uint32_t t = 0; t < numTris; t++)
		{
			for (uint32_t i = 0; i < 3; i++)
			{
				vertIndices.push_back((triIndices[t])[i]);
			}
		}

		// Resize vertices to match number of indices so vertices can be
		// duplicated into the array, successfully rendering models with no indices
		std::vector<PerVertexData> vertices;
		vertices.resize(vertIndices.size(), PerVertexData());

		for (size_t i = 0; i < vertices.size(); i++)
		{
			vertices[i].position = vertPositions[vertIndices[i]];

			if (texCoords.size() > 0)
			{
				vertices[i].texcoord = texCoords[vertIndices[i]];
			}

			if (vertNormals.size() > 0)
			{
				vertices[i].normal = vertNormals[vertIndices[i]];
			}

			vertices[i].color = glm::vec3(1.0f, 1.0f, 1.0f);
		}

#ifdef IMPORTER_DEBUG
		for (size_t v = 0; v < vertices.size(); v++)
		{
			DEBUG_LOG("MD5Importer", LOG_INFO, "Final vertex data [%i] : Pos[%f, %f, %f] : Tex[%f, %f] : Nrm[%f, %f, %f], Clr[%f, %f, %f]",
				v,
				vertices[v].position.x, vertices[v].position.y, vertices[v].position.z,
				vertices[v].texcoord.x, vertices[v].texcoord.y,
				vertices[v].normal.x, vertices[v].normal.y, vertices[v].normal.z,
				vertices[v].color.x, vertices[v].color.y, vertices[v].color.z);
		}
#endif

		verticesOut = vertices;
	}
	else
	{
		// TODO
		DEBUG_LOG("MD5Importer", LOG_ERROR, "Invalid flags parameter!");
		in_file.close();
		return false;
	}
	
	in_file.close();
	DEBUG_LOG("MD5Importer", LOG_SUCCESS, "Imported data from '%s'", path.c_str());
	return true;
}
