#pragma once

#include <iostream>
#include <string>

#include <glew.h>
#include <glfw3.h>

#include <glm.hpp>
#include <vec3.hpp>
#include <mat4x4.hpp>
#include <gtc\matrix_transform.hpp>

enum DIRECTION { FORWARD = 0, BACK, LEFT, RIGHT, UP, DOWN };

class Camera
{
private:
	glm::mat4 _viewMatrix;

	GLfloat _movementSpeed;
	GLfloat _sensitivity;

	glm::vec3 _worldUp;
	glm::vec3 _position;
	glm::vec3 _front;
	glm::vec3 _right;
	glm::vec3 _up;

	GLfloat _pitch;
	GLfloat _yaw;
	GLfloat _roll;

	void _UpdateCameraVectors();

public:
	Camera(glm::vec3 position, glm::vec3 direction, glm::vec3 worldUp);
	~Camera();

	void UpdateInput(const float& deltaTime, const short direction, const double& offsetX, const double& offsetY);

	void Translate(const float& deltaTime, const short direction);
	void Rotate(const float& deltaTime, const double& offsetX, const double& offsetY);

	const glm::mat4 GetViewMatrix();
	const glm::vec3 GetPosition();

	inline void SetMovementSpeed(const float val)
	{
		_movementSpeed = static_cast<GLfloat>(val);
	}

	inline float GetMovementSpeed()
	{
		return static_cast<float>(_movementSpeed);
	}

};