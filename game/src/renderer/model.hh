#pragma once

#include "renderer/shader.hh"
#include "renderer/texture.hh"
#include "renderer/material.hh"
#include "renderer/mesh.hh"
#include "objloader.hh"

class Model
{
private:
	Material* _material;
	Texture* _overrideTextureDiffuse;
	Texture* _overrideTextureSpecular;
	std::vector<Mesh*> _meshes;
	glm::vec3 _position;
public:
	Model(
		glm::vec3 position,
		Material* material,
		Texture* orTexDiff,
		Texture* orTexSpec,
		std::vector<Mesh*>& meshes
	)
	{
		_position = position;
		_material = material;
		_overrideTextureDiffuse = orTexDiff;
		_overrideTextureSpecular = orTexSpec;

		for (auto* i : meshes)
		{
			_meshes.push_back(new Mesh(*i));
		}

		for (auto& i : _meshes)
		{
			i->Translate(_position);
			i->SetOrigin(_position);
		}
	}

	Model(
		glm::vec3 position,
		Material* material,
		Texture* orTexDiff,
		Texture* orTexSpec,
		const char* objFile
	)
	{
		_position = position;
		_material = material;
		_overrideTextureDiffuse = orTexDiff;
		_overrideTextureSpecular = orTexSpec;

		std::vector<Vertex> object = ImportOBJ(objFile);
		_meshes.push_back(new Mesh(object.data(), object.size(), NULL, 0, glm::vec3(1.0f, 0.0f, 0.0f),
			glm::vec3(0.0f),
			glm::vec3(0.0f),
			glm::vec3(1.0f)
		));

		for (auto& i : _meshes)
		{
			i->Translate(_position);
			i->SetOrigin(_position);
		}
	}

	~Model()
	{
		for (auto*& i : _meshes)
			delete i;
	}

	void Translate(const glm::vec3 val)
	{
		for (auto& i : _meshes)
		{
			i->Translate(val);
		}
	}

	void Rotate(const glm::vec3 val)
	{
		for (auto& i : _meshes)
		{
			i->Rotate(val);
		}
	}

	void Scale(const glm::vec3 val)
	{
		for (auto& i : _meshes)
		{
			i->Scale(val);
		}
	}

	void Draw(Shader* shader)
	{
		_material->SendToShader(*shader);

		// Use the program -- should be AFTER calling Set functions from the shader class, since they Use and UnUse the program!!
		shader->Use();

		// Draw
		for (auto& i : _meshes)
		{
			// Activate a texture
			_overrideTextureDiffuse->Bind(0);
			_overrideTextureSpecular->Bind(1);

			i->Draw(shader);
		}
	}
};