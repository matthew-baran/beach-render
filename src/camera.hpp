#pragma once

#include <GLAD/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>

struct MouseState
{
	bool first_mouse = true;
	double last_x = 0;
	double last_y = 0;
};

class CameraState
{
	float camera_speed = 0.05f;

	glm::vec3 camera_pos = glm::vec3(0.0f, 0.0f, 3.0f);
	glm::vec3 camera_front = glm::vec3(0.0f, 0.0f, -1.0f);
	glm::vec3 camera_up = glm::vec3(0.0f, 1.0f, 0.0f);

	MouseState mouse;

	GLFWwindow* window;

public:

	float fov = 45.0f;
	float yaw = -90.0f;
	float pitch = 0.0f;

	CameraState(GLFWwindow* window) : window(window) {}

	glm::vec3 cameraPosition();
	glm::vec3 cameraFront();
	MouseState mouseState();
	glm::mat4 projectionMatrix();
	glm::mat4 viewMatrix();
	GLFWwindow* Window();
	void setCameraFront(glm::vec3 direction);
	void setCameraPosition(glm::vec3 position);
	void setCameraSpeed(float speed);
	void setMouseState(MouseState state);

	void moveForward();
	void moveBackward();
	void moveLeft();
	void moveRight();
};