#pragma once

#include <iostream>
#include <string>
#include <fstream>
#include <filesystem> // Only for checking if file exists
#include <vector>
#include <sstream>
#include <algorithm>
#include <regex>

#include <glew.h>
#include <glfw3.h>

#include <glm.hpp>
#include <vec3.hpp>
#include <vec4.hpp>
#include <mat4x4.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

#include "common.hh"

enum OBJ_INDICES_TYPE { VERTS_ONLY = 0, VERTS_INDICES, VERTS_TEXCOORDS_NORMALS };

/// Returns an array of Vertex structs -- vectors of vertices, texture coordinates (if they exist), and normals (if they exist) -- from an OBJ file.
static bool ImportOBJ(const std::string& filename, std::vector<PerVertexData>& verticesOut) noexcept
{
	// Vertex stuff
	std::vector<glm::fvec3> vertex_positions;
	std::vector<glm::fvec2> vertex_texcoords;
	std::vector<glm::fvec3> vertex_normals;

	// Face stuff
	std::vector<GLint> vertex_position_indices;
	std::vector<GLint> vertex_texcoord_indices;
	std::vector<GLint> vertex_normal_indices;

	std::stringstream ss;
	std::ifstream in_file(filename);
	std::string line = "";
	std::string prefix = "";
	glm::vec3 temp_vec3;
	glm::vec2 temp_vec2;
	GLint temp_glint = 0;

	if (!std::filesystem::exists(filename))
	{
		DEBUG_LOG("OBJLoader", LOG_ERROR, "Could not open [%s] (file does not exist)", filename.c_str());
		return 0;
	}

	if (!in_file.is_open())
	{
		DEBUG_LOG("OBJLoader", LOG_ERROR, "Could not open [%s] (file could not be read)", filename.c_str());
		return 0;
	}

	// OBJ files describe indices in different ways
	// They can either be 'v v v', 'v//n v//n v//n', or 'v/t/n v/t/n v/t/n'
	// Below: figure out what format the file uses
	int8_t format = -1;
	{
		bool finished = false;
		while (std::getline(in_file, line) && !finished)
		{
			ss.clear();
			ss.str(line);
			ss >> prefix;

			if (prefix == "f")
			{
				std::regex hasNoSlashes(R"(^[^/]+$)");
				if (std::regex_search(line, hasNoSlashes))
				{
					format = OBJ_INDICES_TYPE::VERTS_ONLY;
				}
				else
				{
					std::regex hasDoubleSlashes(R"((?:\/\/))");
					if (std::regex_search(line, hasDoubleSlashes))
					{
						format = OBJ_INDICES_TYPE::VERTS_INDICES;
					}
					else
					{
						format = OBJ_INDICES_TYPE::VERTS_TEXCOORDS_NORMALS;
					}
				}

				finished = true;
			}
		}
	}

	/*
	* From people.cs.clemson.edu/~dhouse/courses/405/docs/brief-obj-file-format.html
	*
	* Vertex data:
	* 	v    Geometric vertices:                   v x y z
	* 	vt   Texture vertices:                     vt u v
	* 	vn   Vertex normals:                       vn dx dy dz
	* Elements:
	* 	p    Point:                                p v1
	* 	l    Line:                                 l v1 v2 ... vn
	* 	f    Face:                                 f v1 v2 ... vn
	* 	f    Face with texture coords:             f v1/t1 v2/t2 .... vn/tn
	* 	f    Face with vertex normals:             f v1//n1 v2//n2 .... vn//nn
	* 	f    Face with txt and norms:              f v1/t1/n1 v2/t2/n2 .... vn/tn/nn
	* Grouping:
	* 	g          Group name:                     g groupname
	* Display/render attributes:
	* 	usemtl     Material name:                  usemtl materialname
	* 	mtllib     Material library:               mtllib materiallibname.mtl
	*/

	in_file.clear();
	in_file.seekg(0, in_file.beg); // Send stream to start of file so vertices can be read

	// For every new line in the file...
	while (std::getline(in_file, line))
	{
		// ...get the prefix of the line, and collect values to the correct array
		ss.clear();
		ss.str(line);
		ss >> prefix;

		// TODO: See if there's a better way to handle if/else-if w/ C strings
		if (prefix == "#")
		{
			// Ignore, this is a comment
		}
		else if (prefix == "o")
		{
			// Mesh name
		}
		else if (prefix == "s")
		{
			// Shading groups
		}
		else if (prefix == "use_mtl")
		{

		}
		else if (prefix == "v")
		{
			ss >> temp_vec3.x >> temp_vec3.y >> temp_vec3.z;
			vertex_positions.push_back(temp_vec3);
		}
		else if (prefix == "vt")
		{
			ss >> temp_vec2.x >> temp_vec2.y;
			vertex_texcoords.push_back(temp_vec2);
		}
		else if (prefix == "vn")
		{
			ss >> temp_vec3.x >> temp_vec3.y >> temp_vec3.z;
			vertex_normals.push_back(temp_vec3);
		}
		else if (prefix == "f")
		{
			int counter = 0;
			switch (format)
			{
			case OBJ_INDICES_TYPE::VERTS_ONLY:
				while (ss >> temp_glint)
				{
					vertex_position_indices.push_back(temp_glint);

					if (ss.peek() == ' ')
					{
						ss.ignore(1, ' ');
					}
				}
				break;

			case OBJ_INDICES_TYPE::VERTS_INDICES:
				while (ss >> temp_glint)
				{
					if (counter == 0)
					{
						vertex_position_indices.push_back(temp_glint);
					}
					else if (counter == 1)
					{
						vertex_normal_indices.push_back(temp_glint);
					}

					if (ss.peek() == '/')
					{
						counter = 1;
						ss.ignore(2, EOF);
					}
					else if (ss.peek() == ' ')
					{
						counter = 0;
						ss.ignore(1, ' ');
					}
				}
				break;

			case OBJ_INDICES_TYPE::VERTS_TEXCOORDS_NORMALS:
				while (ss >> temp_glint)
				{
					//Pushing indices into correct arrays
					if (counter == 0)
					{
						vertex_position_indices.push_back(temp_glint);
					}
					else if (counter == 1)
					{
						vertex_texcoord_indices.push_back(temp_glint);
					}
					else if (counter == 2)
					{
						vertex_normal_indices.push_back(temp_glint);
					}

					//Handling characters
					if (ss.peek() == '/')
					{
						++counter;
						ss.ignore(1, '/');
					}
					else if (ss.peek() == ' ')
					{
						++counter;
						ss.ignore(1, ' ');
					}

					//Reset the counter
					if (counter > 2)
						counter = 0;
				}
				break;

			default:
				DEBUG_LOG("OBJLoader", LOG_ERROR, "Could not open [%s] (unknown .OBJ indices formatting)", filename.c_str());
				return 0;
			}
		}
	}

	// Finally, compile final vertex array for mesh...
	std::vector<PerVertexData> vertices;
	vertices.resize(vertex_position_indices.size(), PerVertexData());

	// ... and load in all indices
	for (size_t i = 0; i < vertices.size(); ++i)
	{
		vertices[i].position = vertex_positions[vertex_position_indices[i] - 1]; // OBJ's don't index from 0???

		if (vertex_texcoords.size() > 0)
			vertices[i].texcoord = vertex_texcoords[vertex_texcoord_indices[i] - 1];

		if (vertex_normals.size() > 0)
			vertices[i].normal = vertex_normals[vertex_normal_indices[i] - 1];

		vertices[i].color = glm::vec3(1.0f, 1.0f, 1.0f);
	}

	in_file.close();

	DEBUG_LOG("OBJLoader", LOG_SUCCESS, "Model [%s] loaded in format [%i] with [%i] vertices, [%i] texcoords, and [%i] normals!", filename.c_str(), format, vertex_positions.size(), vertex_texcoords.size(), vertex_normals.size());

	verticesOut = vertices;

	return 1;
}

/// Returns an array of just the position vectors from the vertex data of an OBJ file.
/// Used in algorithms like QuickHull to shrink-wrap models for collision meshes, where texcoords/normals/indices are not needed.
static std::vector<glm::vec3> ImportOBJPositions(const std::string& filename) noexcept
{
	std::vector<glm::vec3> positions;

	std::stringstream ss;
	std::ifstream in_file(filename);
	std::string line = "";
	std::string prefix = "";
	glm::vec3 temp_vec3;
	GLint temp_glint = 0;

	if (!std::filesystem::exists(filename))
	{
		DEBUG_LOG("OBJLoader", LOG_ERROR, "Could not open [%s] (file does not exist)", filename.c_str());
		return positions;
	}

	if (!in_file.is_open())
	{
		DEBUG_LOG("OBJLoader", LOG_ERROR, "Could not open [%s] (file could not be read)", filename.c_str());
		return positions;
	}

	in_file.seekg(0, in_file.beg); // Send stream to start of file so vertices can be read

	// For every new line in the file...
	while (std::getline(in_file, line))
	{
		// ...get the prefix of the line, and collect values to the correct array
		ss.clear();
		ss.str(line);
		ss >> prefix;

		if (prefix == "v")
		{
			ss >> temp_vec3.x >> temp_vec3.y >> temp_vec3.z;
			positions.push_back(temp_vec3);
		}
	}

	in_file.close();

	DEBUG_LOG("OBJLoader", LOG_SUCCESS, "Vertex data from [%s] loaded with [%i] vertices!", filename.c_str(), positions.size());

	return positions;
}
