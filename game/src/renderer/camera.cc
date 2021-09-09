#include "camera.hh"

void Camera::_UpdateCameraVectors()
{
	_front.x = std::cos(glm::radians(_yaw)) * std::cos(glm::radians(_pitch));
	_front.y = std::sin(glm::radians(_pitch));
	_front.z = std::sin(glm::radians(_yaw)) * std::cos(glm::radians(_pitch));

	_front = glm::normalize(_front);
	_right = glm::normalize(
		glm::cross(_front, _worldUp)
	);
	_up = glm::normalize(
		glm::cross(_right, _front)
	);
}

Camera::Camera(glm::vec3 position, glm::vec3 direction, glm::vec3 worldUp)
{
	_viewMatrix = glm::mat4(1.0f);

	_movementSpeed = 3.0f;
	_sensitivity = 200.0f;

	_worldUp = worldUp;
	_position = position;
	_right = glm::vec3(0.0f);
	_up = worldUp;

	_pitch = 0.0f;
	_yaw = -90.0f;
	_roll = 0.0f;

	_UpdateCameraVectors();
}

Camera::~Camera()
{
	
}

const glm::mat4 Camera::GetViewMatrix()
{
	_viewMatrix = glm::lookAt(_position, _position + _front, _up);
	_UpdateCameraVectors();
	return _viewMatrix;
}

const glm::vec3 Camera::GetPosition()
{
	return _position;
}

void Camera::UpdateInput(const float& deltaTime, const short direction, const double& offsetX, const double& offsetY)
{
	this->Rotate(deltaTime, offsetX, offsetY);
	//	this->Translate(deltaTime, direction);
}

void Camera::Translate(const float& deltaTime, const short direction)
{
	switch (direction)
	{
		case DIRECTION::FORWARD:
			_position += _front * _movementSpeed * deltaTime;
			break;
		case DIRECTION::BACK:
			_position -= _front * _movementSpeed * deltaTime;
			break;
		case DIRECTION::LEFT:
			_position -= _right * _movementSpeed * deltaTime;
			break;
		case DIRECTION::RIGHT:
			_position += _right * _movementSpeed * deltaTime;
			break;
		case DIRECTION::UP:
		//	_position += _right * _movementSpeed * deltaTime;
			break;
		case DIRECTION::DOWN:
		//	_position += _right * _movementSpeed * deltaTime;
			break;
		default:
			break;
	}
}

void Camera::Rotate(const float& deltaTime, const double& offsetX, const double& offsetY)
{
	_pitch += static_cast<GLfloat>(offsetY) * _sensitivity * deltaTime;
	_yaw += static_cast<GLfloat>(offsetX) * _sensitivity * deltaTime;

	if (_pitch > 80.0f)
	{
		_pitch = 80.0f;
	}
	else if (_pitch < -80.0f)
	{
		_pitch = -80.0f;
	}

	if (_yaw < -360.0f || _yaw > 360.0f)
	{
		_yaw = 0.0f;
	}
}
