#include "window_management.hpp"

#include <iostream>

namespace msb
{

void framebufferSizeCallback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void mouseCallback(GLFWwindow* window, double xpos, double ypos)
{
    if (glfwGetWindowUserPointer(window) == NULL)
    {
        return;
    }

    auto state = static_cast<CameraState*>(glfwGetWindowUserPointer(window));
    auto mouse = state->mouseState();

    if (mouse.first_mouse)
    {
        mouse.last_x = xpos;
        mouse.last_y = ypos;
        mouse.first_mouse = false;
    }

    auto xoffset = static_cast<float>(xpos - mouse.last_x);
    auto yoffset = static_cast<float>(mouse.last_y - ypos);
    mouse.last_x = xpos;
    mouse.last_y = ypos;
    state->setMouseState(mouse);

    float sensitivity = 0.05f;

    state->yaw += xoffset * sensitivity;
    state->pitch += yoffset * sensitivity;

    if (state->pitch > 89.0f)
    {
        state->pitch = 89.0f;
    }
    if (state->pitch < -89.0f)
    {
        state->pitch = -89.0f;
    }

    glm::vec3 direction;
    direction.x = std::cos(glm::radians(state->yaw)) * std::cos(glm::radians(state->pitch));
    direction.y = std::sin(glm::radians(state->pitch));
    direction.z = std::sin(glm::radians(state->yaw)) * std::cos(glm::radians(state->pitch));
    state->setCameraFront(direction);
}

void scrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    if (glfwGetWindowUserPointer(window) == NULL)
    {
        return;
    }

    auto state = static_cast<CameraState*>(glfwGetWindowUserPointer(window));

    state->fov -= static_cast<float>(yoffset);
    if (state->fov < 1.0f)
    {
        state->fov = 1.0f;
    }
    if (state->fov > 45.0f)
    {
        state->fov = 45.0f;
    }
}

void processInput(CameraState& state)
{
    if (glfwGetKey(state.Window(), GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(state.Window(), true);
    }

    if (glfwGetKey(state.Window(), GLFW_KEY_W) == GLFW_PRESS)
    {
        state.moveForward();
    }

    if (glfwGetKey(state.Window(), GLFW_KEY_S) == GLFW_PRESS)
    {
        state.moveBackward();
    }

    if (glfwGetKey(state.Window(), GLFW_KEY_A) == GLFW_PRESS)
    {
        state.moveLeft();
    }

    if (glfwGetKey(state.Window(), GLFW_KEY_D) == GLFW_PRESS)
    {
        state.moveRight();
    }
}

GLFWwindow* initializeWindow()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    auto window = glfwCreateWindow(800, 600, "OpenGL Window", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        exit(-1);
    }
    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        glfwTerminate();
        exit(-1);
    }

    glViewport(0, 0, 800, 600);

    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
    glfwSetScrollCallback(window, scrollCallback);
    glfwSetCursorPosCallback(window, mouseCallback);

    return window;
}

} // namespace msb