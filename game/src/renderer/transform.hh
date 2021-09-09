#pragma once

#include "camera.hh"

struct Transform
{
private:
	glm::vec3 _position;
	glm::vec3 _origin;
	glm::vec3 _rotation;
	glm::vec3 _scale;
public:
	Transform(
		const glm::vec3& position = glm::vec3(), 
		const glm::vec3& origin = glm::vec3(),
		const glm::vec3& rotation = glm::vec3(), 
		const glm::vec3& scale = glm::vec3(1.0f)
	)
	{
		_position = position;
		_origin = origin;
		_rotation = rotation;
		_scale = scale;
	}

	// my versions
	inline glm::mat4 GetModelMatrix() const
	{
		glm::mat4 m = glm::mat4(1.0f);
		m = glm::translate(m, _origin);
		m = glm::rotate(m, glm::radians(_rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
		m = glm::rotate(m, glm::radians(_rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
		m = glm::rotate(m, glm::radians(_rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
		m = glm::translate(m, _position - _origin);
		m = glm::scale(m, _scale);
		
		return m;
	}

	inline void GetViewMatrix(glm::mat4* memory, Camera* camera) const
	{
		std::cout << "Memory addr of mat4:viewMatrix inside func: " << &memory << "\n";

		*memory = camera->GetViewMatrix();
	}

	inline void GetPerspectiveMatrix(glm::mat4* memory, float& fov, const float& aspectRatio, float& nearZ, float& farZ) const
	{
		std::cout << "Memory addr of mat4:projectionMatrix inside func: " << &memory << "\n";

		*memory = glm::mat4(1.0f); // Reset
		*memory = glm::perspective(glm::radians(fov), aspectRatio, nearZ, farZ);
	}

	// end of my versions
//	inline glm::mat4 GetModel() const
//	{
//		glm::mat4 positionMatrix = glm::translate(_position);
//		glm::mat4 scaleMatrix = glm::scale(_scale);
//		glm::mat4 matRotationX = glm::rotate(_rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
//		glm::mat4 matRotationY = glm::rotate(_rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
//		glm::mat4 matRotationZ = glm::rotate(_rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));
//		glm::mat4 rotationMatrix = matRotationX * matRotationY * matRotationZ;
//
//		return positionMatrix * rotationMatrix * scaleMatrix;
//	}

	inline glm::mat4 GetMVP(Camera& camera) const
	{
		glm::mat4 VP = camera.GetViewMatrix();
		glm::mat4 M = GetModelMatrix();

		return VP * M;
	}

	inline const glm::vec3 GetPosition() { return _position; }
	inline const glm::vec3 GetRotation() { return _rotation; }
	inline const glm::vec3 GetScale() { return _scale; }

	inline void SetPosition(const glm::vec3& pos) { _position = pos; }
	inline void SetRotation(const glm::vec3& rot) { _rotation = rot; }
	inline void SetScale(const glm::vec3& scale) { _scale = scale; }
};