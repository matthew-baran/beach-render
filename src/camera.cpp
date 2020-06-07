#include "camera.hpp"

#include <glm/gtc/matrix_transform.hpp>

void CameraState::setCameraFront(glm::vec3 direction)
{
	camera_front = direction;
}

void CameraState::setCameraPosition(glm::vec3 position)
{
	camera_pos = position;
}

void CameraState::setCameraSpeed(float speed)
{
	camera_speed = speed;
}

GLFWwindow* CameraState::Window()
{
	return window;
}

glm::mat4 CameraState::viewMatrix()
{
	return glm::lookAt(camera_pos, camera_pos + camera_front, camera_up);
}

glm::mat4 CameraState::projectionMatrix()
{
	//return glm::perspective(glm::radians(fov), 800.0f / 600.0f, 0.1f, 100.0f);
	return glm::perspective(glm::radians(fov), 800.0f / 600.0f, 0.1f, 200.0f);
}

glm::vec3 CameraState::cameraPosition()
{
	return camera_pos;
}

glm::vec3 CameraState::cameraFront()
{
	return camera_front;
}

void CameraState::moveForward()
{
	camera_pos += camera_speed * camera_front;
	//camera_pos.y = 0;
}

void CameraState::moveBackward()
{
	camera_pos -= camera_speed * camera_front;
	//camera_pos.y = 0;
}

void CameraState::moveLeft()
{
	camera_pos -= camera_speed * glm::normalize(glm::cross(camera_front, camera_up));
	//camera_pos.y = 0;
}

void CameraState::moveRight()
{
	camera_pos += camera_speed * glm::normalize(glm::cross(camera_front, camera_up));
	//camera_pos.y = 0;
}

MouseState CameraState::mouseState()
{
	return mouse;
}

void CameraState::setMouseState(MouseState state)
{
	mouse = state;
}