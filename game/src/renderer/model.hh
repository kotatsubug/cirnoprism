#pragma once

#include "shader.hh"
#include "texture.hh"
#include "material.hh"
#include "mesh.hh"
#include "../objloader.hh"

class Model
{
private:
	Material* material;
	Texture* overrideTextureDiffuse;
	Texture* overrideTextureSpecular;
	std::vector<Mesh*> meshes;
	glm::vec3 position;
public:
	Model(
		glm::vec3 position,
		Material* material,
		Texture* orTexDiff,
		Texture* orTexSpec,
		std::vector<Mesh*>& meshes
	)
	{
		this->position = position;
		this->material = material;
		this->overrideTextureDiffuse = orTexDiff;
		this->overrideTextureSpecular = orTexSpec;

		for (auto* i : meshes)
		{
			this->meshes.push_back(new Mesh(*i));
		}

		for (auto& i : this->meshes)
		{
			i->Translate(this->position);
			i->SetOrigin(this->position);
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
		this->position = position;
		this->material = material;
		this->overrideTextureDiffuse = orTexDiff;
		this->overrideTextureSpecular = orTexSpec;

		std::vector<Vertex> object = LoadOBJ(objFile);
		this->meshes.push_back(new Mesh(object.data(), object.size(), NULL, 0, glm::vec3(1.0f, 0.0f, 0.0f),
			glm::vec3(0.0f),
			glm::vec3(0.0f),
			glm::vec3(1.0f)
		));

		for (auto& i : this->meshes)
		{
			i->Translate(this->position);
			i->SetOrigin(this->position);
		}
	}

	~Model()
	{
		for (auto*& i : this->meshes)
			delete i;
	}

	void Translate(const glm::vec3 val)
	{
		for (auto& i : this->meshes)
		{
			i->Translate(val);
		}
	}

	void Rotate(const glm::vec3 val)
	{
		for (auto& i : this->meshes)
		{
			i->Rotate(val);
		}
	}

	void Scale(const glm::vec3 val)
	{
		for (auto& i : this->meshes)
		{
			i->Scale(val);
		}
	}

	void Draw(Shader* shader)
	{
		this->material->SendToShader(*shader);

		// Use the program -- should be AFTER calling Set functions from the shader class, since they Use and UnUse the program!!
		shader->Use();

		// Draw
		for (auto& i : this->meshes)
		{
			// Activate a texture
			this->overrideTextureDiffuse->Bind(0);
			this->overrideTextureSpecular->Bind(1);

			i->Draw(shader);
		}
	}
};