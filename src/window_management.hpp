#pragma once

#include "camera.hpp"

#include <GLAD/glad.h>
#include <GLFW/glfw3.h>

namespace msb
{

GLFWwindow* initializeWindow();
void processInput(CameraState& state);

} // namespace msb